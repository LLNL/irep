# This should be included by makefiles that need to build IREP WKT libraries.
#
# To build a wkt library, e.g., `libfoo-wkt.a`, simply include this file,
# define a variable `foo.wkt_src`, and add the WKT library as a make target.
# For example:
#
#     include /path/to/wkt.mk
#     foo.wkt_src = wkt_shapes.h wkt_colors.h wkt_numbers.h
#     all: libfoo-wkt.a
#
# The wkt library will be generated and built from the provided WKT headers.
#

# utility program for generating code from wkt.h files
irep_generate = bin/irep-generate

# Rules for compiling C and Forran files -- note that the appropriate
# IREP -D flag must be set for files that include IREP headers.
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -DIREP_LANG_C       -I.
COMPILE.f = $(FC) $(FFLAGS) $(CPPFLAGS) -DIREP_LANG_FORTRAN -I.
%.o: %.c       ; $(COMPILE.c) -c $<
%.o %.mod: %.f ; $(COMPILE.f) -c $<

# override gmake's default assumption that .mod is a modula file
# without this, you'll get warnings about cicular dependencies and
# gmake will try to run m2c on module files.
%.o: %.mod

# generate fortran from every wkt_%.h file
wkt_%.f: wkt_%.h
	$(irep_generate) --mode fortran $< > $@

# These functions define the pieces of a wkt library.
# They're defined as functions so they can be used as prerequisites
# in our doubly expanded pattern rule for WKT libs below.
ir-gen  = $(filter %.h,$(foreach v,$($(1).wkt_src),$(if $(findstring wkt_,$v),$v)))
ir-wkt  = $(filter wkt_%,$(notdir $(call ir-gen,$(1))))
ir-wobj = $(subst .h,.o,$(call ir-wkt,$(1)))

# add a little flair to the output
cgreen = "\033[1;32m"
cend    = "\033[0m"

# Any target like libfoo-wkt.a will be built like a wkt library.
# The user needs only define foo.wkt_src to make the magic happen.
.SECONDEXPANSION:
lib%-wkt.a: $$(call ir-wobj,$$*)
	$(AR) -rc $@ $^
	$(RANLIB) $@
	@echo -e Successfully created IREP WKT library $(cgreen)$@$(cend).
