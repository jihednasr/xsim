# Gnuplot script. Launch with "gnuplot file.plot"
# Uncomment to generate png file
# set terminal png
# set output 'speed_up.png'

# Uncomment to generate pdf file
# set terminal postscript enhanced color
# set output '| ps2pdf - speed_up.pdf'

#set title "speed-up"
#set view 60,30
set style data linespoints
set pointsize 1
set mouse
set xlabel "time window"
set xtics 10
set ytics 10
set ylabel "time info frequence"
set zlabel "speed-up"
set pm3d


splot "measures.gp" using 1:2:3 notitle

pause -1
