#The template for generating multiple plots
#sharing the same legend in the same figure 

#The figure setting (applied to all plots)
#=========================================

#Origin file format
# col1  col2  col3  col4       col5      col6
#      fixedS KAlgo LQT-Oracle SE-Oracle unfixedS
# pdf
# eps
# eps
# eps
# eps

# set terminal pdf monochrome enhanced fname  "Helvetica" fsize 32 
set output "parameter-range-1.eps"
set terminal postscript portrait enhanced mono "Helvetica" 32 

# set size 5.1000, 0.800000
set pointsize 4.000000



# set multiplot layout 1,3
#The first plot (which generates the common legend)
#===========================================================
# #Note the NaN variable.

# set size 5.10000,0.100000;  
# set origin -0.15,0.72;

# set key inside top center samplen 2 width 5
# set key horizontal spacing 1

# set yrange [0.0000000001:0.0000000002]

# unset border
# unset tics
# set bmargin 1
# #unset label

# plot NaN title 'CheetahGIS' with linespoints linetype 1 pointtype 2, \
# NaN title 'CheetahGIS^-' with linespoints linetype 1 pointtype 4, \
# NaN title 'Baseline' with linespoints linetype 1 pointtype 8
# # NaN title 'SE-Oracle' with linespoints linetype 1 pointtype 10, \
# # NaN title 'LQT-Oracle' with linespoints linetype 1 pointtype 6#, \
# # NaN title 'Theoretical bound ({/Symbol e})' with lines linetype 2
# #NaN title 'Hybrid' with linespoints linetype 1 pointtype 6, \
# #, \NaN title 'RN-Adapt' with linespoints linetype 1 pointtype 4, \
# #NaN title 'Cao-Appro2-Adapt2' with linespoints linetype 1 pointtype 12, \
# #NaN title 'Long-Appro-Adapt2' with linespoints linetype 1 pointtype 14
 
# set border
# set tics
# set label

#The 1nd plot (notitle)
#=========================================
set size 1.68000,0.8000000;  
set origin 0.0,0.0;

set xlabel  "Query area ratio ({/Symbol \264}10^{-2}%)"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key left

set xrange [2: 64]
set yrange [10:40000]
# set label 11 center at graph 0.5,char 1 "(a) Range count query, Shopping" 
# set bmargin 5
# set format x "%2.0t{/Symbol \264}10^{%L}"
set format y "%2.0t"
# set xtics font ", 25"
set xtics ( "1{/Symbol \264}10^{-2}" 0.01,"4{/Symbol \264}10^{-2}" 0.04, "16{/Symbol \264}10^{-2}" 0.16)

set log x; set xtics 2,2,64
# set log y
set ytics 0,10000,40000

# set key bottom height 1.5
set key top right
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "performance-atc-rangequery.txt" using 1:4   title 'CheetahGIS' with linespoints linetype 1 pointtype 2, \
"performance-atc-rangequery.txt" using 1:2  title 'CheetahGIS^-' with linespoints linetype 1 pointtype 4, \
"performance-atc-rangequery.txt" using 1:3  title 'Baseline'  with linespoints linetype 1 pointtype 8


set output "parameter-range-2.eps"
#The 2nd plot (notitle)
#=========================================
set size 1.68000,0.8000000;  
set origin 0.0,0.0;

set xlabel  "Query area ratio ({/Symbol \264}10^{-9}%)"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key left

set xrange [2: 64]
set yrange [10:50000]
# set label 11 center at graph 0.5,char 1 "(b) Range count query, GeoLife" 
# set bmargin 5
set format x "%.0s"
set format y "%2.0t"
# set xtics font ", 25"
set xtics ( "1{/Symbol \264}10^{-2}" 0.01,"4{/Symbol \264}10^{-2}" 0.04, "16{/Symbol \264}10^{-2}" 0.16)

set log x
set xtics 2, 2, 64
# set log y
set ytics 0,10000,50000

# set key bottom height 1.5
set key top right
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "performance-geo-rangequery.txt" using 1:4  title 'CheetahGIS' with linespoints linetype 1 pointtype 2, \
"performance-geo-rangequery.txt" using 1:2   title 'CheetahGIS^-' with linespoints linetype 1 pointtype 4, \
"performance-geo-rangequery.txt" using 1:3   title 'Baseline'  with linespoints linetype 1 pointtype 8


set output "parameter-range-3.eps"
#The 3nd plot (notitle)
#=========================================
set size 1.68000,0.8000000;  
set origin 0.0,0.0;

set xlabel  "Query area ratio ({/Symbol \264}10^{-5}%)"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key left

set xrange [16: 512]
set yrange [10:50000]
# set label 11 center at graph 0.5,char 1 "(c) Range count query, Brinkhoff" 
# set bmargin 5
# set format x "%.0s"
set format y "%2.0t"
# set xtics font ", 25"
# set xtics ( "1{/Symbol \264}10^{-2}" 0.01,"4{/Symbol \264}10^{-2}" 0.04, "16{/Symbol \264}10^{-2}" 0.16)

set log x
set xtics 16, 2, 512
# set log y
set ytics 0,10000,50000

# set key bottom height 1.5
set key top right

# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "performance-brk-rangequery.txt" using 1:4   title 'CheetahGIS'  with linespoints linetype 1 pointtype 2, \
"performance-brk-rangequery.txt" using 1:2  title 'CheetahGIS^-' with linespoints linetype 1 pointtype 4, \
"performance-brk-rangequery.txt" using 1:3   title 'Baseline' with linespoints linetype 1 pointtype 8

# #The 2nd plot (notitle)
# #=========================================
# set size 1.25000,0.6000000;  
# set origin 1.28,0.0;

# set xlabel  "k"
# set ylabel  "Query throughput ({/Symbol \264}10^{4} qps)"
# set key left

# set xrange [2: 64]
# set yrange [10:30000]
# set label 11 center at graph 0.5,char 1 "(b) kNN query" 
# set bmargin 5
# set format x "%.0s"
# set format y "%2.0t"
# # set xtics font ", 25"
# # set xtics ( "5{/Symbol \264}10^3" 5000, "1{/Symbol \264}10^4" 10000, "2{/Symbol \264}10^4" 20000, "4{/Symbol \264}10^4" 40000)

# set log x; set xtics 2,2,64
# # set log y
# set ytics 0,10000,30000

# # plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# # "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
# plot "performance-atc-knnquery.txt" using 1:4  notitle with linespoints linetype 1 pointtype 2, \
# "performance-atc-knnquery.txt" using 1:2  notitle with linespoints linetype 1 pointtype 4, \
# "performance-atc-knnquery.txt" using 1:3  notitle with linespoints linetype 1 pointtype 8



# #The 3nd plot (notitle)
# #=========================================
# set size 1.25000,0.6000000;  
# set origin 2.56,0.0;

# set xlabel  "Request throughput (rps)"
# set ylabel  "Avg. latency (ms)"
# set key left

# set xrange [2000: 32000]
# set yrange [10:1000000]
# set label 11 center at graph 0.5,char 1 "(c) Range query" 
# set bmargin 5
# set format x "10^{%T}"
# set format y "10^{%T}"
# # set xtics font ", 25"
# set xtics ( "2{/Symbol \264}10^3" 2000, "4{/Symbol \264}10^3" 4000, "8{/Symbol \264}10^3" 8000, "16{/Symbol \264}10^3" 16000, "32{/Symbol \264}10^3" 32000)

# set log x
# set log y

# # plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# # "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
# plot "conc-rangequery.txt" using 1:4  notitle with linespoints linetype 1 pointtype 2, \
# "conc-rangequery.txt" using 1:2  notitle with linespoints linetype 1 pointtype 4, \
# "conc-rangequery.txt" using 1:3  notitle with linespoints linetype 1 pointtype 8



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