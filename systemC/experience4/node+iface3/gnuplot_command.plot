# Gnuplot script. Launch with "gnuplot file.plot"
# Uncomment to generate png file
# set terminal png
# set output 'speed_up.png'

# Uncomment to generate pdf file
# set terminal postscript enhanced color
# set output '| ps2pdf - speed_up.pdf'

#set title "speed-up"
set style data linespoints
set pointsize 1
set mouse
set xlabel "number of node"
set xtics 1
set ytics 1
set ylabel "number of interface"
set zlabel "speed-up"

set grid
set grid ztics
set ztics out
set ticslevel 0
set border 1+2+4+8+16+32+64+256+512
set view 60,30
set pm3d


splot "measures.gp" using 1:2:3 notitle

pause -1
