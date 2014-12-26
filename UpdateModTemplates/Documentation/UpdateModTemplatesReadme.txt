Getting started and general overview
====================================

This program performs a bulk update of placed instances in a module, copying
data from blueprints back to instances.  It is intended to be used to make bulk
changes to large numbers of placed objects when you have since edited the
original blueprint.  The program is designed to receive a list of object
type(s) and blueprint name(s) of placed objects that are to be updated in the
module.  All areas are updated, and the process should only take a few seconds
to complete.

When an object instance is updated, all data in the blueprint replaces the
data in the object instance.  Data that is present in the object instance but
not the blueprint (i.e. position information) is kept as-is.  GFF lists are
replaced and not merged (for example, creature inventories).

To update object instances in your module, follow these steps:

1) Make a back-up copy of your module, in case you need to revert your changes.

2) Find the location of your NWN2 install directory (typically under
   C:\Program Files\Atari\Neverwinter Nights 2).

3) Find the location of your NWN2 home directory (typically under your user
   documents directory, such as
   C:\Users\<username>\Documents\Neverwinter Nights 2).

4) Save your module as a directory mode module under the Modules directory in
   your chosen NWN2 home directory.

5) Close the toolset (if it is open).  You should not have the module loaded
   into the toolset when you run the program, or the toolset will overwrite any
   changes made on the next save.

6) Open a command prompt (Start->Run->cmd.exe), then change to the directory
   where UpdateModTemplates.exe resides (with cd "directory name").

7) Run the UpdateModTemplates program with the chosen object type and blueprint
   names.  For exampe, if you have a module named "My Module", and you want to
   update the blueprints for all creatures with blueprint c_ancom_badger1, you
   might use a command line similar to the following:

   UpdateModTemplates -home "%userprofile%\My Documents\Neverwinter Nights 2" -installdir "%ProgramFiles%\Atari\Neverwinter Nights 2" -module "My Module" -template c_ancom_badger1 -objecttype creature

   Note that any argument with spaces must be enclosed in double quotes.  You
   may specify any number of templates and object types to update at the same
   time.

   The -module argument should only have the name of the module directory given
   and not the whole path to it.

Command line arguments listing
==============================

Running the program with no arguments will display the valid command line usage
for the program:

UpdateModTemplates

This program copies data from the specified template(s) to placed object
instances of the given object type(s) within a directory mode module.
Only directory mode modules are supported!

Optionally, a list of GFF fields may be excluded from updating via the
usage of the -excludefield parameter (for example,
-excludefield ModelScale).  Use a GFF editor to pick the fields to
exclude (if desired).

Usage: UpdateModTemplates -home <homedir> -installdir <installdir>
                          -module <module resource name> [-nwn1]
                          -template <first template name to update>
                          [-template <additional template name N...>]
                          -objecttype <first object type to match>
                          [-objecttype <additional object type N...>]
                          [-excludefield <exclude field 1...>]

Legal object types are:
   tree
   waypoint
   placedeffect
   placeable
   door
   item
   environmentobject
   light
   creature
   trigger
   sound
   staticcamera
   store


Advanced usage
==============

To edit a NWN1 module (instead of a NWN2 module), supply the -nwn1 argument.

To restrict the copying of fields from blueprints, use the -excludefield <name>
argument.  This argument (which may be repeated any number of times) will cause
the program to not copy a specific field from the blueprint.  You can use a GFF
viewer or editor program to discover field names -- BioWare's GFF Editor
program can be used to this end.


Support
=======

Visit the NWN2 Community Relations channel at irc.nwn2source.net / #nwn2cr for
assistance, should you get stuck.
