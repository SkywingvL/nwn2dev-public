#
# This makefile include sets up infrastructure to integrate NWScript script
# compilation into the build environment.
#
# Sub-projects must define NSS_FLAVOR=<flavor> and then include this file,
# e.g. NSS_FLAVOR=NWN2 to use the NWN2 includes.
#

NSS_FLAGS=$(NSS_FLAGS) -x "error " -q -e -v1.69 -i $(NSS_INC_PATH) -i $(NSS_INC_PATH)\$(NSS_FLAVOR) -i $(NSS_INCLUDES)

!if !$(USE_OBJECT_ROOT)

{$(TARGET_DIRECTORY)\}.nss${O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $$*.ncs

{}.nss${O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $$*.ncs

{..\$(TARGET_DIRECTORY)\}.nss{$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $$*.ncs

{..\}.nss:$(TARGET_DIRECTORY)\}.nss:${O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $$*.ncs

${O}.nss${O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $$*.ncs

!endif

!if $(USE_OBJECT_ROOT)

{$(TARGET_DIRECTORY)\}.nss{$(OBJ_PATH)\$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $*.ncs

{}.nss{$(OBJ_PATH)\$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $*.ncs

{..\$(TARGET_DIRECTORY)\}.nss{$(OBJ_PATH)\$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $*.ncs

{..\}.nss{$(OBJ_PATH)\$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $*.ncs

{$(OBJ_PATH)\$O}.nss{$(OBJ_PATH)\$O}.ncs:
	set BUILDMSG=(NWScript) Compiling - $<
	$(NSS_COMPILER) $(NSS_FLAGS) $< $*.ncs

!endif

.SUFFIXES: .nss .ncs

#
# The sources file (or highest level makefile.inc) defines the list of NWScript
# object files (*.ncs) that are built in NSS_OBJECTS= before this include file
# is invoked.
#

$(OBJ_PATH)\$O\ScriptSrc: $(NSS_OBJECTS)


