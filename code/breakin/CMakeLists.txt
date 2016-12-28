# Copyright (C) 2016 by B. Nikolic / U. Cambridge. License: GPL3
#
# This file is not yet an offical part of CASA

casa_add_library( breakin
  pythonfns.c
  synthkernels.c)

install (FILES
	pythonfns.h
	DESTINATION include/casacode/breakin
	)



