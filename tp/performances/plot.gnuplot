set term pngcairo size 1024,720
set output "plot.png"
set title "Temps d'exécution moyen de 'cat' en fonction de la taille du tampon choisie"
set logscale x
#set logscale y
set xlabel "Taille du tampon" offset 0, -2.0
set ylabel "Temps d'exécution (en secondes)"
set xtics out offset 0,-3.0
set xtics rotate by 90
set grid x lt 1 lw .75 lc "gray"
plot "exec_time.dat" using 1:2:xtic(gprintf("%.0b%Bo", stringcolumn(1))) title "Réel" with lines,\
     "exec_time.dat" using 1:3:xtic(gprintf("%.0b%Bo", stringcolumn(1))) title "Utilisateur" with lines,\
     "exec_time.dat" using 1:4:xtic(gprintf("%.0b%Bo", stringcolumn(1))) title "Système" with lines
pause -1  "Hit return to continue"
