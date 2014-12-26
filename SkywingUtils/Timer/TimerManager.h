/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	TimerManager.h

Abstract:

	This module defines the TimerManager object, which keeps track of a list of
	timers to fire periodically.  The TimerManager is designed for single
	threaded operation in an I/O dispatch loop.

--*/

#ifndef _SOURCE_PROGRAMS_SKYWINGUTILS_TIMER_TIMERMANAGER_H
#define _SOURCE_PROGRAMS_SKYWINGUTILS_TIMER_TIMERMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace swutil
{

class TimerManager;
class TimerRegistration;

//
// The TimerRegistration object represents a registered timer.  On deletion,
// the object releases the timer registration.
//

//
// The TimerManager object allows timers to be registered and deregistered.  It
// is intended to be called as part of an I/O dispatcher loop to control the
// wait timeout cycle and manage timers as a part of that control mechanism.
//

class TimerManager
{

public:

	typedef swutil::SharedPtr< TimerManager > Ptr;

	TimerManager(
		);

	~TimerManager(
		);

	//
	// Define the timer completion routine.
	//
	// If the routine returns false, then this indicates that the timer has
	// been canceled.  The user must not cancel a timer within the timer
	// routine and return true if doing so will allow the TimerRegistration
	// object to be deleted.
	//

	typedef
	bool
	(__stdcall * OnTimerCompletionProc)(
		__in void * Context1,
		__in void * Context2,
		__in TimerRegistration * Timer
		);

	//
	// Timer registration.  The TimerRegistration object may be deleted to
	// clear the timer registration.
	//
	// N.B.  All timers MUST be deregistered before the TimerManager object is
	//       deleted.
	//

	swutil::SharedPtr< TimerRegistration >
	CreateTimer(
		__in OnTimerCompletionProc TimerCompletionRoutine,
		__in void * TimerContext1,
		__in void * TimerContext2
		);

	//
	// Cancel a timer.  This routine is generally never directly invoked but is
	// called by the TimerRegistration destructor.
	//
	// Once a timer has been canceled it may never be used again.
	//

	void
	CancelTimer(
		__in TimerRegistration * Registration
		);

	//
	// This routine runs down active timers.  It is intended to be called once
	// during each dispatch loop cycle.  The return value indicates how long
	// the dispatch cycle may idle waiting for I/O to complete.
	//

	ULONG
	RundownTimers(
		);

	//
	// This internal only routine is called by a TimerRegistration when its
	// timer period changes.  It invalidates the next expiration cache.
	//

	void
	InvalidateTimerExpiration(
		__in TimerRegistration * Timer,
		__in ULONG Period,
		__in ULONG Epoch
		);

	//
	// This routine is called when a timer registration is activated.  It moves
	// the timer to the active list.
	//

	void
	OnTimerRegistrationActivate(
		__in TimerRegistration * Timer
		);

	//
	// This routine is called when a timer registration is deactivated.  It
	// moves a timer to the inactive list.
	//

	void
	OnTimerRegistrationInactivate(
		__in TimerRegistration * Timer
		);

private:

	//
	// Define the inactive timer list, to which all TimerRegistration objects
	// that are inactive are linked.
	//

	LIST_ENTRY           m_TimerListHead;

	//
	// Define the active timer list, to which all active TimerRegistration
	// objects are linked.
	//

	LIST_ENTRY           m_ActiveTimerListHead;

	//
	// Define the mutated flag used to keep track of whether a timer was added
	// or deleted during the dispatch cycle.
	//

	bool                 m_ListMutated;

	//
	// Define the cached next timer expiration time.  This allows us to avoid
	// re-running the list down each time an I/O completes unless we are sure
	// that at least one timer will have expired.
	//
	// m_NextExpiration represents a count of millisecond from
	// m_NextExpirationEpoch at which the next timer in line to expire shall
	// require dispatching.
	//

	ULONG               m_NextExpiration;
	ULONG               m_NextExpirationEpoch;
	bool                m_NextExpirationInvalid;
	TimerRegistration * m_NextExpirationTimer;

};

//
// The TimerRegistration object represents a registered timer instance.  The
// instance supports automatic cleanup on destruction.
//
// A TimerRegistration may be active, deactivated, or canceled.  An active
// timer registration fires periodically.  A deactivated timer registration
// does not general timer events, but may be activated later.  A canceled timer
// is permanently deactivated.
//

class TimerRegistration
{

public:

	typedef swutil::SharedPtr< TimerRegistration > Ptr;

	//
	// Check whether the timer is active.
	//

	inline
	const
	bool
	IsActive(
		) const
	{
		return (m_TimerPeriod != 0);
	}

	//
	// Deactivate the timer.
	//

	inline
	void
	Deactivate(
		)
	{
		if (!IsCanceled( ))
		{
			//
			// If we have a nonzero timer period, then set us to the zero timer
			// period, which has a special meaning:  The timer is deactivated.
			//
			// External users must always use Deactivate to inactivate a timer.
			//

			if (IsActive( ))
				SetPeriod( 0 );
		}
	}

	//
	// Change the timer period.  Changing the period also resets the timer
	// epoch to the current time.
	//
	// Legal timer periods have values other than 0 or INFINITE (0xFFFFFFFF).
	//

	inline
	void
	SetPeriod(
		__in ULONG Period
		)
	{
		bool WasActive;

		if (IsCanceled( ))
			return;

		WasActive = (IsActive( ));

		m_TimerEpoch  = GetTickCount( );
		m_TimerPeriod = Period;

		if (WasActive != IsActive( ))
		{
			if (WasActive)
				m_TimerManager->OnTimerRegistrationInactivate( this );
			else
				m_TimerManager->OnTimerRegistrationActivate( this );
		}

		m_TimerManager->InvalidateTimerExpiration(
			this,
			Period,
			m_TimerEpoch);
	}

	//
	// Stop a timer and return the amount of time left.  The API can only be
	// used on active timers.  The time left is expressed as a count of
	// milliseconds.
	//
	// If zero is returned then the timer has elapsed (but will not have fired
	// because the timer dispatcher did not see it).  In this case, the timer
	// dispatch routine will not be invoked directly.
	//

	inline
	ULONG
	StopTimer(
		)
	{
		ULONG Now;

		if (!IsActive( ))
			return INFINITE;

		Now = GetTickCount( );

		//
		// If we have not expired, return the delta to the expiration time to
		// the caller.
		//

		if ((Now - m_TimerEpoch) < m_TimerPeriod)
		{
			ULONG Remaining = m_TimerPeriod - (Now - m_TimerEpoch);

			SetPeriod( 0 );

			return Remaining;
		}

		//
		// Otherwise indicate that the timer had expired but has not fired yet
		// by returning zero.
		//

		SetPeriod( 0 );
		return 0;
	}

private:

	//
	// Create a new TimerRegistration.  For use only by the TimerManager.
	//

	TimerRegistration(
		__in TimerManager & TimerMgr,
		__in TimerManager::OnTimerCompletionProc TimerCompletionRoutine,
		__in void * TimerContext1,
		__in void * TimerContext2
		);

public:

	~TimerRegistration(
		);

private:

	//
	// Attempt to dispatch an active timer.  The routine returns the count of
	// milliseconds until the next dispatch is required, else INFINITE if the
	// timer was deactivated.
	//

	ULONG
	Dispatch(
		__in ULONG Now
		);

	//
	// Cancel the timer.  This routine is only called by the TimerManager.
	//

	inline
	void
	Cancel(
		)
	{
		if (IsCanceled( ))
			return;

		//
		// Unlink us from the timer manager and set ourselves to the canceled
		// state.
		//

		RemoveEntryList( &m_TimerLinks );

		//
		// Mark us as canceled by setting the timer period to INFINITE.
		//

		m_TimerPeriod = INFINITE;
	}

	//
	// Link the timer to the timer manager's timer list.
	//

	inline
	void
	LinkTimer(
		__in PLIST_ENTRY List
		)
	{
		InsertTailList( List, &m_TimerLinks );

		//
		// Mark us as inactive by setting the timer period to zero.
		//

		m_TimerPeriod = 0;
	}

	//
	// Check if the timer is canceled.
	//

	inline
	bool
	IsCanceled(
		) const
	{
		return (m_TimerPeriod == INFINITE);
	}

private:

	//
	// Define the back link to the timer manager, used to invalidate cached
	// expiration times.
	//

	TimerManager                        * m_TimerManager;

	//
	// Define the list links used to connect the timer to the timer manager
	// object.
	//

	LIST_ENTRY                            m_TimerLinks;

	//
	// Define the count of milliseconds between each timer cycle.
	//

	ULONG                                 m_TimerPeriod;

	//
	// Define the tick count epoch at which the timer last issued a timer cycle
	// or which the timer cycle otherwise begins from.
	//

	ULONG                                 m_TimerEpoch;

	//
	// Define the timer callback procedure to be invoked on timer completion.
	//

	TimerManager::OnTimerCompletionProc   m_TimerCallback;

	//
	// Define the two timer callback parameters, typically a per-instance and
	// per-operation context.
	//

	void                                * m_TimerContext1;
	void                                * m_TimerContext2;

	friend class TimerManager;
};

}

#endif
