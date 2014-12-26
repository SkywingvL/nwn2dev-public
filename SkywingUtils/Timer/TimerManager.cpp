/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TimerManager.cpp

Abstract:

	This module houses the TimerManager object, which supports timer
	registration and expiration for single-threaded I/O dispatcher loops.

--*/

#include "Precomp.h"
#include "../Ref/Ref.h"
#include "../Synchronization/ListAPI.h"
#include "TimerManager.h"

using namespace swutil;

TimerManager::TimerManager(
	)
/*++

Routine Description:

	This routine constructs a new TimerManager.

Arguments:

	None.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_ListMutated( false ),
  m_NextExpiration( 0 ),
  m_NextExpirationEpoch( 0 ),
  m_NextExpirationInvalid( true ),
  m_NextExpirationTimer( NULL )
{
	InitializeListHead( &m_TimerListHead );
	InitializeListHead( &m_ActiveTimerListHead );
}

TimerManager::~TimerManager(
	)
/*++

Routine Description:

	This routine deletes the current TimerManager object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// Forcibly clean up any lingering timers.
	//

	while (!IsListEmpty( &m_TimerListHead ))
	{
		PLIST_ENTRY         ListEntry;
		TimerRegistration * Timer;

		ListEntry = m_TimerListHead.Flink;

		Timer = CONTAINING_RECORD(
			ListEntry,
			TimerRegistration,
			m_TimerLinks);

		CancelTimer( Timer );
	}

	while (!IsListEmpty( &m_ActiveTimerListHead ))
	{
		PLIST_ENTRY         ListEntry;
		TimerRegistration * Timer;

		ListEntry = m_ActiveTimerListHead.Flink;

		Timer = CONTAINING_RECORD(
			ListEntry,
			TimerRegistration,
			m_TimerLinks);

		CancelTimer( Timer );
	}
}

swutil::SharedPtr< TimerRegistration >
TimerManager::CreateTimer(
	__in OnTimerCompletionProc TimerCompletionRoutine,
	__in void * TimerContext1,
	__in void * TimerContext2
	)
/*++

Routine Description:

	This routine allocates a new timer and initializes it.  The timer is linked
	in but is inactivated.

	Once the user is ready to use the timer, it can be activated via a call to
	the TimerRegistration::SetPeriod routine.

Arguments:

	TimerCompletionRoutine - Supplies the completion routine to be invoked when
	                         the timer period elapses.

	TimerContext1 - Supplies the first timer user context argument.

	TimerContext2 - Supplies the second timer user context argument.

Return Value:

	The routine returns a shared pointer to the new TimerRegistration.  The
	TimerRegistration automatically deregisters itself on deletion.  It is the
	caller's responsibility to ensure that all timers are canceled before the
	TimerManager is deleted.

Environment:

	User mode.

--*/
{
	TimerRegistration * Timer;

	Timer = new TimerRegistration(
		*this,
		TimerCompletionRoutine,
		TimerContext1,
		TimerContext2);

	if (Timer == NULL)
		throw std::bad_alloc( );

	Timer->LinkTimer( &m_TimerListHead );

	return Timer;
}

void
TimerManager::CancelTimer(
	__in TimerRegistration * Timer
	)
/*++

Routine Description:

	This routine permanently deactivates a registered timer.  Once in the
	canceled state, a timer will never generate any timer events.

Arguments:

	Timer - Supplies the timer object to cancel.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	Timer->Cancel( );
}

ULONG
TimerManager::RundownTimers(
	)
/*++

Routine Description:

	This routine is called periodically by the user's I/O dispatch loop to
	execute any pending timers.  The routine should be called at least as
	frequently as its return value (expressed in a count of milliseconds).

	The caller must invoke RundownTimers again before entering into a dispatch
	sleep if any timer periods have changed.

Arguments:

	None.

Return Value:

	The routine returns the count of milliseconds before RundownTimers must be
	called again.  If the routine returns the constant INFINITE, then there are
	no timers active.

Environment:

	User mode.

--*/
{
	PLIST_ENTRY         ListEntry;
	bool                Rescan;
	ULONG               NextExpiration;
	ULONG               Now;
	TimerRegistration * NextExpirationTimer;

	Now = GetTickCount( );

	//
	// First, check if our cached next expiration time is still valid.  If so,
	// and we have not yet reached the next expiration time, then we do not
	// need to run the timer list down.
	//

	if (!m_NextExpirationInvalid)
	{
		if (m_NextExpiration == INFINITE)
			return INFINITE;

		//
		// If we have not yet reached the expiration time, then just return the
		// cached time.
		//

		if ((Now - m_NextExpirationEpoch) < m_NextExpiration)
			return m_NextExpiration - (Now - m_NextExpirationEpoch);
	}

	//
	// Loop dispatching timers.  Normally, we take only one pass through the
	// timer list, but if the user adds or deletes a timer while we are in the
	// dispatch cycle, we will start over from the beginning.  This is a rare
	// case and not something we optimize for, for simplicity.
	//

	do
	{
		m_ListMutated       = false;
		Rescan              = false;
		NextExpiration      = INFINITE;
		NextExpirationTimer = NULL;
		Now                 = GetTickCount( );

		for (ListEntry =  m_ActiveTimerListHead.Flink;
		     ListEntry != &m_ActiveTimerListHead;
		     ListEntry = ListEntry->Flink)
		{
			TimerRegistration * Timer;
			ULONG               TimerExpiration;

			Timer = CONTAINING_RECORD(
				ListEntry,
				TimerRegistration,
				m_TimerLinks);

			if (!Timer->IsActive( ))
				continue;

			//
			// Dispatch the timer.
			//

			TimerExpiration = Timer->Dispatch( Now );

			if (TimerExpiration < NextExpiration)
			{
				NextExpiration      = TimerExpiration;
				NextExpirationTimer = Timer;
			}

			//
			// If the user added or removed a timer, which should be very rare
			// during timer dispatching, then start over from the beginning.
			//

			if (m_ListMutated)
			{
				m_ListMutated = false;
				Rescan        = true;
				break;
			}
		}
	} while (Rescan == true);

	//
	// Cache the next time until we need to run down expirations and return to
	// the caller, having completed our dispatch run.
	//

	m_NextExpiration        = NextExpiration;
	m_NextExpirationEpoch   = Now;
	m_NextExpirationInvalid = false;
	m_NextExpirationTimer   = NextExpirationTimer;

	return m_NextExpiration;
}

void
TimerManager::InvalidateTimerExpiration(
	__in TimerRegistration * Timer,
	__in ULONG Period,
	__in ULONG Epoch
	)
/*++

Routine Description:

	This internal only routine is called by timer registration objects when
	they invalidate their expiration intervals.  It invalidates the cache of
	the next timer to expire, but only if necessary.

	A simple cache is used:  We store the timer object closest to expiration,
	and when it will expire.  If the timer is the timer object closest to
	expiration then we must invalidate the cache.  Additionally, if the timer
	object expires sooner than the current expiration, then the cache is
	updated accordingly.

Arguments:

	Timer - Supplies the timer object that has changed its timer expiration.

	Period - Supplies the period of the timer.

	Epoch - Supplies the tick count from which the period is relative to.

Return Value:

	None.

Environment:

	User mode, called by TimerRegistration::SetPeriod only.

--*/
{
	ULONG NextTimeLeft;

	if (m_NextExpirationInvalid)
		return;

	//
	// If we are transitioning from no timers to an active timer then we must
	// throw away the cache.
	//

	if (m_NextExpiration == INFINITE)
	{
		m_NextExpirationInvalid = true;
		return;
	}

	//
	// If the next expiration is already immediate, then there is nothing to
	// do.
	//

	if ((Epoch - m_NextExpirationEpoch) >= m_NextExpiration)
	{
		m_NextExpirationInvalid = true;
		return;
	}

	NextTimeLeft = m_NextExpiration - (Epoch - m_NextExpirationEpoch);

	if (m_NextExpirationTimer == Timer)
	{
		if ((NextTimeLeft < Period)          ||
		    (!Timer->IsActive( ))            ||
		    (Timer->IsCanceled( )))
		{
			//
			// We are extending the expiration of the next timer further into
			// the future.  Our cache is only one level deep so we must
			// recalculate and re-run the timer list next time.
			//

			m_NextExpirationInvalid = true;
			return;
		}
	}
	else
	{
		if ((NextTimeLeft > Period) &&
		    (Timer->IsActive( )))
		{
			//
			// We are changing the next expiration timer to this new timer.  It
			// is not immediately expiring.
			//

			m_NextExpiration      = Period;
			m_NextExpirationEpoch = Epoch;
			m_NextExpirationTimer = Timer;
		}
		else
		{
			//
			// This timer expires after the next expiration timer, and it is
			// not the next expiration timer.  The cache can stay valid.
			//
			// Or, this timer has been disabled or canceled, but it was not the
			// next expiration timer, so the cache can stay valid.
			//

			return;
		}
	}
}

void
TimerManager::OnTimerRegistrationActivate(
	__in TimerRegistration * Timer
	)
/*++

Routine Description:

	This routine handles a timer being activated.  The timer is moved to the
	active timer list.

Arguments:

	Timer - Supplies the timer that is being activated.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	RemoveEntryList( &Timer->m_TimerLinks );
	InsertTailList( &m_ActiveTimerListHead, &Timer->m_TimerLinks );

	m_ListMutated = true;
}

void
TimerManager::OnTimerRegistrationInactivate(
	__in TimerRegistration * Timer
	)
/*++

Routine Description:

	This routine handles a timer being inactivated.  The timer is moved to the
	inactive timer list.

Arguments:

	Timer - Supplies the timer that is being inactivated.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	RemoveEntryList( &Timer->m_TimerLinks );
	InsertTailList( &m_TimerListHead, &Timer->m_TimerLinks );

	m_ListMutated = true;
}

TimerRegistration::TimerRegistration(
	__in TimerManager & TimerMgr,
	__in TimerManager::OnTimerCompletionProc TimerCompletionRoutine,
	__in void * TimerContext1,
	__in void * TimerContext2
	)
/*++

Routine Description:

	This routine constructs a new TimerRegistration.

Arguments:

	TimerCompletionRoutine - Supplies the completion routine to be invoked when
	                         the timer period elapses.

	TimerContext1 - Supplies the first timer user context argument.

	TimerContext2 - Supplies the second timer user context argument.

Return Value:

	None.  Raises an std::exception on failure.

Environment:

	User mode.

--*/
: m_TimerManager( &TimerMgr ),
  m_TimerPeriod( INFINITE ),
  m_TimerEpoch( 0 ),
  m_TimerCallback( TimerCompletionRoutine ),
  m_TimerContext1( TimerContext1 ),
  m_TimerContext2( TimerContext2 )
{
}

TimerRegistration::~TimerRegistration(
	)
/*++

Routine Description:

	This routine deletes the current TimerRegistration object and its associated
	members.

Arguments:

	None.

Return Value:

	None.

Environment:

	User mode.

--*/
{
	//
	// First deactivate the timer if it was active, so that the next timer
	// expiration is updated properly if we were the next to expire.
	//

	if (IsActive( ))
		Deactivate( );

	//
	// Cancel the timer so that no more timer events are fired, and the timer
	// manager does not reference us any more.
	//

	Cancel( );
}

ULONG
TimerRegistration::Dispatch(
	__in ULONG Now
	)
/*++

Routine Description:

	This routine dispatches the timer, checking for timer expiration.  Should
	the timer have expired, the timer epoch is reset and the user callback is
	invoked.

Arguments:

	Now - Supplies the timer expiration epoch (i.e. current tick count).

Return Value:

	The routine returns the count of milliseconds until the timer next expires.

	If the routine returns INFINITE, then the timer has been deactivated.

Environment:

	User mode, timer active and called from timer dispatcher only.

--*/
{
	//
	// If we have not expired, return the delta to the expiration time to the
	// timer dispatcher.
	//

	if ((Now - m_TimerEpoch) < m_TimerPeriod)
		return m_TimerPeriod - (Now - m_TimerEpoch);

	//
	// Otherwise, run user's timer callback after resetting the timer epoch.
	//

	m_TimerEpoch = Now;

	//
	// N.B.  Returning false indicates that the user has deleted the timer
	//       registration object and so we must return immediately in that
	//       case.

	if (!m_TimerCallback( m_TimerContext1, m_TimerContext2, this ))
		return INFINITE;

	//
	// Return the new next time until expiration.
	//

	if ((!IsActive( )) || (IsCanceled( )))
		return INFINITE;
	else
		return m_TimerPeriod;
}
