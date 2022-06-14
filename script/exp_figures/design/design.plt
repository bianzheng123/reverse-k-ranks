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

set output "design-1.eps" # TODO
set terminal postscript portrait enhanced mono "Helvetica" 32 

# set size 5.1000, 0.80000
set pointsize 4.000000

# set multiplot layout 1,4
#The first plot (which generates the common legend)
#===========================================================
#Note the NaN variable.

# set size 5.00000,0.100000;  
# set origin -0.15,0.72;

# set key inside top center samplen 2 width 5
# set key horizontal spacing 1

# set yrange [0.0000000001:0.0000000002]

# unset border
# unset tics
# set bmargin 1
# #unset label

# plot NaN title 'CheetahGIS' with linespoints linetype 1 pointtype 2, \
# NaN title 'One to one' with linespoints linetype 1 pointtype 15, \
# NaN title 'Disable exclusive task slot' with linespoints linetype 1 pointtype 16, \
# NaN title 'Disable load balance' with linespoints linetype 1 pointtype 17
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
set size 1.25000,0.8000000;  
set origin 0,0.0;

set xlabel  "Query area ratio ({/Symbol \264}10^{-2} %)"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key right

set xrange [2: 64]
set yrange [10:40000]
# set label 11 center at graph 0.5,char 1 "(a) Range count query, Shopping" 
# set bmargin 5
set format x "%.0s"
set format y "%2.0t"
# set xtics font ", 25"
# set xtics ( "5{/Symbol \264}10^3" 5000, "1{/Symbol \264}10^4" 10000, "2{/Symbol \264}10^4" 20000, "4{/Symbol \264}10^4" 40000)

set log x; set xtics 2,2,64
# set log y
# set ytics 1,10,100000
set ytics 0,10000,40000

set key bottom right
set key samplen 2
set key font ",28"
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "design-atc-rangequery(for-rm).txt" using 1:3 title 'RM-enabled in CheetahGIS' with linespoints linetype 1 pointtype 2, \
"design-atc-rangequery(for-rm).txt" using 1:2  title 'RM-disabled in CheetahGIS' with linespoints linetype 1 pointtype 16

set output "design-2.eps" # TODO

#The 2nd plot (notitle)
#=========================================
set size 1.25000,0.8000000;  
set origin 0,0.0;

set xlabel  "k"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key right

set xrange [2: 64]
set yrange [10:30000]
# set label 11 center at graph 0.5,char 1 "(b) kNN query, Shopping" 
# set bmargin 5
set format x "%.0s"
set format y "%2.0t"
# set xtics font ", 25"
# set xtics ( "5{/Symbol \264}10^3" 5000, "1{/Symbol \264}10^4" 10000, "2{/Symbol \264}10^4" 20000, "4{/Symbol \264}10^4" 40000)

set log x; set xtics 2,2,128
# set log y
# set ytics 1,10,100000
set ytics 0,10000,30000


set key bottom right
set key samplen 2
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "design-atc-knnquery(for-rm).txt" using 1:3 title 'RM-enabled in CheetahGIS' with linespoints linetype 1 pointtype 2, \
"design-atc-knnquery(for-rm).txt" using 1:2  title 'RM-disabled in CheetahGIS' with linespoints linetype 1 pointtype 16

set output "design-3.eps" # TODO

unset key

#The 3nd plot (notitle)
#=========================================
set size 1.25000,0.8000000;  
set origin 0,0.0;

set xlabel  "Query area ratio ({/Symbol \264}10^{-2} %)"
set ylabel  "Throughput ({/Symbol \264}10^4)"
set key right

set xrange [64: 2048]
set yrange [0:25000]
# set label 11 center at graph 0.5,char 1 "(c) Range count query, Shopping" 
# set bmargin 5
# set format x "%2.0t{/Symbol \264}10^{%L}"
set format x "%.0f"
set format y "%2.0t"
# set xtics font ", 25"
# set xtics ( "1{/Symbol \264}10^{-2}" 0.005,"4{/Symbol \264}10^{-2}" 0.04, "16{/Symbol \264}10^{-2}" 0.16)

set log x
set xtics 64,2,2048
# set log y
set ytics 0,10000,30000

set key bottom left 
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "design-atc-range(for-cg).txt" using 1:3  title 'many-to-one' with linespoints linetype 1 pointtype 2, \
"design-atc-range(for-cg).txt" using 1:2  title 'one-to-one'  with linespoints linetype 1 pointtype 14

set output "design-4.eps" # TODO

#The 4nd plot (notitle)
#=========================================
set size 1.25000,0.8000000;  
set origin 0,0.0;

set xlabel  "k"
set ylabel  "Throughput ({/Symbol \264}10^3)"
set key right

set xrange [2: 64]
set yrange [0:3000]
# set label 11 center at graph 0.5,char 1 "(d) kNN query, synthetic bias dataset" 
# set bmargin 5
set format x "%.0s"
set format y "%2.0t"
# set xtics font ", 25"
# set xtics ( "5{/Symbol \264}10^3" 5000, "1{/Symbol \264}10^4" 10000, "2{/Symbol \264}10^4" 20000, "4{/Symbol \264}10^4" 40000)

set log x; set xtics 2,2,64
# set log y
set ytics 0,1000,4000


set key bottom  
set key height 3
# plot "indexTime.res" using 1:5  notitle with linespoints linetype 1 pointtype 10, \
# "indexTime.res" using 1:6 notitle with linespoints linetype 1 pointtype 6
plot "design-atcbias-knnquery(for-lb).txt" using 1:3  title 'CheetahGIS with LB' with linespoints linetype 1 pointtype 2, \
"design-atcbias-knnquery(for-lb).txt" using 1:2  title 'CheetahGIS without LB' with linespoints linetype 1 pointtype 12


unset multiplot

set output