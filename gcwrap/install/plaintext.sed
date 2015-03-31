#!/bin/bash
echo '\documentclass[landscape]{article}'
echo '\usepackage{hyperref}'
echo '\usepackage{aips2help}'
echo '\input{aips2defs}'
echo '\providecommand{\n}{\newline}'
echo '\providecommand{\aipspp}{CASA table}'
echo '\providecommand{\tool}{tool}'
echo '\begin{document}'
echo '\pagenumbering{gobble}'
echo '\setlength{\textwidth}{8in} '
echo '\setlength{\linewidth}{8in} '
echo '\begin{verbatim}'
sed -e "s/<description>/<description>\\\end{verbatim}/g" $1 | sed -e "s/<\/description>/\\\begin{verbatim}<\/description>/g"
echo '\end{verbatim}'
echo '\end{document}'