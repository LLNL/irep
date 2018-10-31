# Copyright (c) 2016, Lawrence Livermore National Security, LLC. Produced
# at the Lawrence Livermore National Laboratory.  Written by Lee Busby,
# busby1@llnl.gov. LLNL-CODE-702338. All rights reserved.
# See ../../Copyright for additional notices.
#
all: irep.3.pdf irep-intro.pdf

irep.3.pdf: irep.3
	man -t ./$^ | ps2pdf - - > $@

irep.3: irep.pod
	pod2man -s3 -r"IREP 2.0" -c"The IREP Library" $? > $@

irep-%.pdf: irep-%.tr
	groff -t -p -mm $^ | ps2pdf - - > $@

clean: ; rm -f irep.3 *.pdf
