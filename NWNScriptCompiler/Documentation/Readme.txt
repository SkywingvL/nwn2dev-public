NWNScriptCompiler is an improved version of Edward T. Smith (Torlack's)
nwnnsscomp, with numerous bugfixes and improvements.  It is a standalone,
console driven compiler suitable for batch usage outside of the toolset.

Improvements over the stock nwnnsscomp include:

- New -l option to load resources from the game zip files.
- New -b option for true batch mode under Windows.
- New -r option to load module.ifo from any path.
- Nested structs are now usable and do not produce an erroneous syntax error.
- Case statement blocks now obey proper rules for variable declarations; it is
  no longer possible to declare a variable that is skipped by a case statement,
  which would not compile with the standard (toolset) script compiler.
- The main and StartingConditional symbols are now required to be functions.
- Prototypes that do not match their function declarations are now fixed up by
  the compiler (with a warning) if compiler version 1.69 or lower is specified.
  This allows several broken include scripts shipped with the game to compile
  Note that a prototype cannot be fixed up if a function is called before the
  real declaration is processed.  In this case, an error is generated; the
  standard compiler generates bad code in such a case.  A prototype that is
  fixed up by the compiler generates a compiler warning.
- Uninitialized constants (const int x;) now generate a warning instead of an
  error if compiler version 1.69 or lower is specified.  The standard script
  compiler silently permits such constructs.
- An error is generated if a string literal's length exceeds 511 characters and
  compiler version 1.69 or lower is specified.  This is consistent with the
  standard script compiler's internal limitations.
- Attempts to use a switch statement while immediately located within a
  do/while block now generate a warning.  In the standard script compiler, such
  constructs cause bad code generation due to a compiler bug.
- Nested structure element access now generates a warning.  In the standard
  script compiler, a compiler bug may result in compilation errors or bad code
  generation with nested structure usage.
- Modifying a constant global variable before it is declared now generates a
  warning.  In nwnnsscomp, such constructs could either cause an internal
  compiler assertion failure, or could result in bad code generation.
- Script disassembly now generates a high level IR output (.ir), and a high
  level optimized IR output (.ir-opt).
- Scripts can now be loaded directly from a module.
- NWN1 game resource directories no longer are required to be present on the
  Windows build.
- NWN1-style resource directories can be used with the -1 command line option.
- Error diagnostics relating to the "const" qualifier now provide more context
  information about the problematic identifier.
- Local variables with the "const" qualifier now correctly trigger a diagnostic
  instead of an internal compiler assertion.
- In version 1.69 or lower, the compiler now issues a warning diagnostic if a
  script program has a scope where the sum of all global and local identifiers
  (compile time constants exempted) which are visible exceeds 1024.  This
  condition produces a script that cannot be compiled with the standard
  compiler.
- The compiler now issues a warning diagnostic if a script program attempts to
  illegally reference global variables before main runs, instead of silently
  emitting bad code.  The standard compiler emits bad code in this condition.
- The compiler now issues a warning diagnostic if a script program attempts to
  make use of an expression involving the "action" type before main runs,
  instead of silently emitting bad code.  The standard compiler emits bad code
  in this condition.
- STORE_STATE references emitted before main no longer leak uninitialized data
  to the compiled script instruction stream.
- Response files, listing one argument per line, are now supported with the
  @ResponseFile option.
- In version 1.69 or lower, the compiler issues a warning diagnostic if a
  function has greater than 32 arguments.  The standard compiler fails to
  compile such scripts.
- The compiler now issues an error diagnostic if an identifier is declared with
  multiple incompatible types (i.e. as both a struct and variable).  Previously
  this would cause an internal compiler assertion.
- The compiler no longer generates an internal compiler assertion if a script
  program explicitly calls the entry point symbol (i.e. main).
- Compiler diagnostics relating to default argument misuse now provide better
  context information about the problematic identifier.
- The compiler now issues a warning diagnostic if a script program attempts to
  provide default arguments to an entry point symbol (i.e. main).  The runtime
  environment does not honor compile time default arguments for entry point
  symbols.
- The compiler now correctly issues an error diagnostic if a script program
  attempts to call a function that has been declared but never defined, instead
  of raising an internal compiler assertion.
- The compiler provides support for 'default functions' in extensions mode,
  if -e is supplied on the compiler command line.  A default function can have
  no body and if called, the compiler will automatically substitute a default
  implementation that throws the arguments away and returns the appropriate
  default value for any return value(s) as appropriate.  This feature can be
  activated via a #pragma default_function(function-identifier) directive after
  a prototype for "function-identifier" is observed, but before the function is
  defined.  If a default function is defined as normal, the script program's
  explicit definition is used as the function body instead of the default
  definition.
- The compiler now supports automatic analysis and verification of scripts
  as a compilation post-step in order to check for various inconsistencies.
  Script analysis and verification can be enabled via the -a command line
  option at a modest (10-20%) cost to compile time on most scripts.
- The compiler supports __FILE__, __LINE__, __COUNTER__, __FUNCTION__,
  __NSC_COMPILER_DATE__, and __NSC_COMPILER_TIME__ predefined macros.  The
  macros work the same as their typical C counterparts (with the compiler
  macros reflecting the build time for the compiler itself).  These macros are
  only enabled in extensions mode.
- The compiler now supports #error and #warning to issue diagnostics to the
  programmer.  These directives are only enabled in extensions mode.
- The compiler now has limited #if/#elif/#ifdef/#ifndef/#else/#endif support.
  Only literal integers and macros can currently be tested; there is no support
  for expressions on preprocessor #ifs constructs.  The preprocessor #if
  constructs are only enabled in extensions mode.
- The compiler no longer generates bad code when optimizations are enabled, a
  global variable is initialized with a function call expression, and the
  global variable in question is never written to after assignment.
- In compatibility mode, a warning is now issued if a script program contains a
  for statement with an initialzer or increment block whose type is not an
  integer.
- Error and warning diagnostic messages now have a stable set of message
  identifiers (NSCnnnn where 'nnnn' is a four-digit number), for easier
  searching.
- The compiler now stops at the first error unless -y is specified.  If the -y
  flag is specified, then a count of errors is shown at the end.
- The compiler now correctly issues an error diagnostic if multiple bodies are
  provided for the same function and all of the function bodies are empty.  The
  previous, erroneous and nonconforming behavior was to permit such code
  declarations.
- Diagnostics relating to duplicate identifier definitions now generally
  contain a source location pointer to the first definition.
- Script disassembly IR output now includes the literal values of constant
  variables referenced.
- New warning NSC6021 issued in compatibility mode when a nested assigment
  where the RHS expression being assigned is itself an assignment, e.g.
  "a = b = c;", is encountered.  This does not parse properly in the
  standard compiler.
- A longstanding bug dating back to nwnnsscomp, where global variables that
  were readonly and had initializers with assignments might be improperly
  inlined if optimizations were enabled, has been fixed.
- The compiler now optimizes dead sub-expressions in simple cases, such as
  multiplication by zero or shift by 32 bits.
- The compiler now optimizes multiply operations into shifts by power of 2 when
  possible.
- The compiler now optimizes add one or subtract one operations into the faster
  increment or decrement operation when possible.
- The compiler now issues warning NSC6023 if prototypes specify default
  constant literal values for a default argument on a given function.
- The compiler now allows functions to be marked as having no side effects via
  the #pragma pure_function(function-identifier) pragma.  This hints to the
  optimizer that the function call can be elided (or even inlined multiple
  times).
- The compiler now fixes up programs that include an include file with an
  unterminated comment block (in compatibility mode only, v1.69 or below).
- The compiler now allows compiled scripts up to 64MB (versus the old 512K
  limit).  Some scripts were exceeding this internal limit.  It is still highly
  recommended that scripts be kept much smaller than this, size, however.
- The compiler now correctly handles scripts that exceed the maximum compiled
  script size with a proper compile time error (NSC1036).
- The compiler now supports a -j option to show where includes are being
  handled from (useful for seeing where a file was really drawn from if there
  are multiple resource providers that could provide that include file).
- The compiler now correctly handles disambiguating between struct tag and
  variable symbols.
- The compiler now supports a -k option to generate a pseudo-preprocessed
  version of script source text, annotated with originating line numbers and
  file names (useful for diagnosing problems with conflicting #includes).
- The compiler now issues a diagnostic error NSC1089 if a duplicate case select
  value was supplied within a switch block.  This matches behavior with the
  BioWare compiler (instead of silently generating code for an unreachable case
  scan block).
Run NWNScriptCompiler -? for a listing of command line options and their
meanings.  Existing nwnnsscomp options are preserved and kept functional.

