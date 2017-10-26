all : report.pdf

clean :
	rm report.aux report.log report.dvi report.ps report.toc

report.pdf : report.tex
	pslatex report.tex
	pslatex report.tex
	dvips report.dvi
	ps2pdf report.ps report.pdf
