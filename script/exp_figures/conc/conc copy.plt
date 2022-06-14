#The template for generating multiple plots
#sharing the same legend in the same figure 

#The figure setting (applied to all plots)
#=========================================

#Origin file format
# col1  col2  col3  col4       col5      col6
#      fixedS KAlgo LQT-Oracle SE-Oracle unfixedS
# eps
# eps
# eps
# eps
# eps

set output "conc-1.eps"
set terminal postscript portrait enhanced mono "Helvetica" 32 

# set size 5.1000, 0.800000
set pointsize 4.000000

set style data histogram
set style histogram clustered gap 1
set style fill solid 0.4 border
set key samplen 1
set key width -1

# set multiplot layout 2,4
#The first plot (which generates the common legend)
#===========================================================
#Note the NaN variable.

# set size 5.00000,0.100000;  
# set origin -0.15,0.62;

# set key inside top center samplen 2 width 5
# set key horizontal spacing 1

# set yrange [0.0000000001:0.0000000002]

# unset border
# unset tics
# set bmargin 1
# #unset label

# plot NaN title 'CheetahGIS' lt 1 with boxes fs pattern 6, \
# NaN title 'CheetahGIS^-' lt 1 with boxes fs pattern 3, \
# NaN title 'Baseline' lt 1 with boxes fs pattern 5

# plot NaN title 'THOR' with linespoints linetype 1 pointtype 2, \
# NaN title 'NO\_META' with linespoints linetype 1 pointtype 4, \
# NaN title 'QUERY\_BCAST' with linespoints linetype 1 pointtype 8
# NaN title 'SE-Oracle' with linespoints linetype 1 pointtype 10, \
# NaN title 'LQT-Oracle' with linespoints linetype 1 pointtype 6#, \
# NaN title 'Theoretical bound ({/Symbol e})' with lines linetype 2
#NaN title 'Hybrid' with linespoints linetype 1 pointtype 6, \
#, \NaN title 'RN-Adapt' with linespoints linetype 1 pointtype 4, \
#NaN title 'Cao-Appro2-Adapt2' with linespoints linetype 1 pointtype 12, \
#NaN title 'Long-Appro-Adapt2' with linespoints linetype 1 pointtype 14
 
set border
set tics
set label

#The 1nd plot (notitle)
#=========================================

set size 1.25000,0.6000000;  
set origin 0.0,0.0;

set xlabel  "# of data updates per second ({/Symbol \264}10^4)"
set ylabel  "Avg. latency (ms)"
set key top left
set key maxrows 3
# unset key

# set xrange [10000: 160000]
set autoscale xfix

set yrange [10:1000]
# set label 11 center at graph 0.5,char 1 "(a) Data update, Shopping" 
# set bmargin 5
# set format x "10^{%T}"
set format y "10^{%T}"
# set xtics font ", 25"

# set log x
set log y

plot "conc-update.txt" using 4:xticlabels(1)  title 'CheetahGIS'  fs pattern 6, \
"conc-update.txt" using 2:xticlabels(1) title 'CheetahGIS^-'  fs pattern 3, \
"conc-update.txt" using 3:xticlabels(1) title 'Baseline' fs pattern 5

# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
# plot "conc-update.txt" using 1:4  notitle with linespoints linetype 1 pointtype 2, \
# "conc-update.txt" using 1:2  notitle with linespoints linetype 1 pointtype 4, \
# "conc-update.txt" using 1:3  notitle with linespoints linetype 1 pointtype 8

set output "conc-2.eps"

#The 2nd plot (notitle)
#=========================================

set size 1.25000,0.6000000;  
set origin 0,0.0;

set xlabel  "# of queries per second ({/Symbol \264}10^3)"
set ylabel  "Avg. latency (ms)"
set key top left
set key maxrows 3

# set xrange [10000: 160000]
set autoscale xfix

set yrange [10:200000]
# set label 11 center at graph 0.5,char 1 "(b) Object query, Shopping" 
# set bmargin 5
# set format x "10^{%T}"
set format y "10^{%T}"
# set xtics font ", 25"

# set log x
set log y

plot "conc-objectquery.txt" using 4:xticlabels(1)  title 'CheetahGIS'  fs pattern 6, \
"conc-objectquery.txt" using 2:xticlabels(1) title 'CheetahGIS^-'  fs pattern 3, \
"conc-objectquery.txt" using 3:xticlabels(1) title 'Baseline'  fs pattern 5

set output "conc-3.eps"

#The 3nd plot (notitle)
#=========================================

set size 1.25000,0.6000000;  
set origin 0,0.0;

set xlabel  "# of queries per second ({/Symbol \264}10^3)"
set ylabel  "Avg. latency (ms)"
set key top left
set key maxrows 3

# set xrange [10000: 160000]
set autoscale xfix

set yrange [10:1000000]
# set label 11 center at graph 0.5,char 1 "(c) Range count query, Shopping" 
# set bmargin 5
# set format x "10^{%T}"
set format y "10^{%T}"
# set xtics font ", 25"

# set log x
set log y

plot "conc-rangequery.txt" using 4:xticlabels(1)  title 'CheetahGIS'  fs pattern 6, \
"conc-rangequery.txt" using 2:xticlabels(1) title 'CheetahGIS^-'  fs pattern 3, \
"conc-rangequery.txt" using 3:xticlabels(1) title 'Baseline'  fs pattern 5

set output "conc-4.eps"
#The 4nd plot (notitle)
#=========================================

set size 1.25000,0.6000000;  
 set origin 0,0.0;

set xlabel  "# of queries per second ({/Symbol \264}10^2)"
set ylabel  "Avg. latency (ms)"
set key top left
set key maxrows 3

# set xrange [10000: 160000]
set autoscale xfix

set yrange [10:500000000]
# set label 11 center at graph 0.5,char 1 "(d) kNN query, Shopping" 
# set bmargin 5
# set format x "10^{%T}"
set format y "10^{%T}"
# set xtics font ", 25"

# set log x
set log y

plot "conc-knnquery.txt" using 4:xticlabels(1)  title 'CheetahGIS'  fs pattern 6, \
"conc-knnquery.txt" using 2:xticlabels(1) title 'CheetahGIS^-'  fs pattern 3, \
"conc-knnquery.txt" using 3:xticlabels(1) title 'Baseline'  fs pattern 5

# #The 3nd plot (notitle)
# #=========================================
# set size 1.25000,0.6000000;  
# set origin 3.84,0.0;

# set xlabel  "Request throughput (rps)"
# set ylabel  "Avg. latency (ms)"
# set key left

# set xrange [100: 1600]
# set yrange [10:1000000]
# set label 11 center at graph 0.5,char 1 "(d) kNN query" 
# set bmargin 5
# set format x "10^{%T}"
# set format y "10^{%T}"
# # set xtics font ", 25"
# set xtics ( "1{/Symbol \264}10^2" 100, "2{/Symbol \264}10^2" 200, "4{/Symbol \264}10^2" 400, "8{/Symbol \264}10^2" 800, "16{/Symbol \264}10^2" 1600)

# set log x
# set log y

# # plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# # "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
# plot "conc-knnquery.txt" using 1:4  notitle with linespoints linetype 1 pointtype 2, \
# "conc-knnquery.txt" using 1:2  notitle with linespoints linetype 1 pointtype 4, \
# "conc-knnquery.txt" using 1:3  notitle with linespoints linetype 1 pointtype 8


unset multiplot

set output