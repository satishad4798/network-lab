
set terminal png  size 1020,720
set output "satish.png"

set title "health Data"
set xdata time  
set timefmt "%d-%m-%Y"
set format x "%d-%m"
set xtics rotate by 90 offset 0,-2 out nomirror
set xlabel "time"

set ylabel "skipped"
plot "mydata.txt" using 3:2 with linespoints title "time diff t2-t1" lw 2, 7 title "Minimum difference"



