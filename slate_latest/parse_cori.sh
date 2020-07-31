#!/bin/bash
PANEL_NUM=4
if [ "$1" = "getrf" ] || [ "$1" = "geqrf" ]; then
    for app in $1 #getrf geqrf
    do
        for prec in d #c
        do
            for size in `seq $2 $4 $3`
            do
                for lookahead in 1 #2 #4 8
                do
                    for num in $PANEL_NUM #21 #1 7 14 21 28
                    do
                        CUR_RESULT=
                        for impl in org hclib
                        do
                            if [ -f log_${impl}_${app} ]; then
#                                echo 'grep "$prec.*host.*\s$size\s .* $lookahead \s* $num \s* [0-9]*\." log_${impl}_${app} | awk '{print $(NF-4);}' | sort -n | awk '{if (NR>1) print $(NF)}''
                                grep "$prec.*host.*\s$size" log_${impl}_${app} | awk '{print $(NF-4);}' | sort -n | awk '{if (NR>1) print $(NF)}' > temp_${impl}
                                CUR_RESULT="$CUR_RESULT, `awk 'BEGIN{s=0;}{s=s+$1;}END{print s/NR;}' temp_${impl}`,`awk '{delta = $1 - avg; avg += delta / NR; mean2 += delta * ($1 - avg); } END { print sqrt(mean2 / (NR-1));}' temp_${impl}`"
                            fi
#                            CUR_RESULT="$CUR_RESULT, `grep "$prec.*host.*$size .* $lookahead \s* $num \s* [0-9]*\." log_${impl}_${app} | awk '{print $(NF-3);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==3) med=$NF; else if(NR==7) max=$NF;} END {print min","med","max}'`"
                        done
#grep "$prec.*host.*$size .* 1 \s* NA" log_ref_${app}
                        for impl in ref_mpi #ref ref_mpi
                        do
                            if [ -f log_${impl}_${app} ]; then
                                grep "$prec.*host.*\s$size\s .* 1 \s* 1 \s* NA" log_${impl}_${app} | awk '{print $(NF-2);}' | sort -n | awk '{if (NR>1) print $NF}' > temp_${impl}
                                CUR_RESULT="$CUR_RESULT, `awk 'BEGIN{s=0;}{s=s+$1;}END{print s/NR;}' temp_${impl}`,`awk '{delta = $1 - avg; avg += delta / NR; mean2 += delta * ($1 - avg); } END { print sqrt(mean2 / (NR-1));}' temp_${impl}`"
                            fi
                        done

#                        awk '{if (NR==1) min=$NF; else if(NR==3) med=$NF; else if(NR==7) max=$NF;} END {print min","med","max}'`"

                        echo "$app, $prec, $size, $size, $lookahead, $num $CUR_RESULT" #`grep "$prec.*$size .* $num" log_hclib_${app} | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==5) med=$NF; else if(NR==10) max=$NF;} END {print min,med,max}'`"
#        echo "$size, $size, $num, `grep "$size.* $num" $1 | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==5) med=$NF; else if(NR==10) max=$NF;} END {print min,med,max}'`"
#    grep "$size.* 14" $1 | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==3) print $NF}'
                    done
                done
            done
        done
    done
elif [ "$1" = "potrf" ]; then
    for app in $1 
    do
        for prec in d
        do
            for size in `seq $2 $4 $3`
            do
                for lookahead in 1 #4 8
                do
                    CUR_RESULT=
                    for impl in org hclib
                    do
                        if [ -f log_${impl}_${app} ]; then

#                        echo "grep "$prec.*host.*$size\s.*[0-9]*\s*[0-9]*\s*$lookahead\s*[0-9]*\..*e""
#                         echo "grep "$prec.*host.*\s$size\s.*[0-9]*\s*[0-9]*\s*$lookahead\s*[0-9]*\..*e" log_${impl}_${app} | awk '{ print $(NF-4);}' | sort -n | awk '{if(NR>1) print $NF;}'"

                            grep "$prec.*host.*\s$size" log_${impl}_${app} | awk '{ print $(NF-4);}' | sort -n | awk '{if(NR>1) print $NF;}' > temp_${impl}
                            CUR_RESULT="$CUR_RESULT, `awk 'BEGIN{s=0;}{s=s+$1;}END{print s/NR;}' temp_${impl}`,`awk '{delta = $1 - avg; avg += delta / NR; mean2 += delta * ($1 - avg); } END { print sqrt(mean2 / (NR-1));}' temp_${impl}`"
                        fi

#                        CUR_RESULT="$CUR_RESULT, `grep "$prec.*host.*$size.*[0-9]*\s*[0-9]*\s*$lookahead\s*[0-9]*\..*e" log_${impl}_${app} | awk '{print $(NF-3);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==5) med=$NF; else if(NR==10) max=$NF;} END {print min","med","max}'`"
                    done
                    for impl in ref ref_mpi
                    do
                        if [ -f log_${impl}_${app} ]; then
                            grep "$prec.*host.*\s$size\s.*[0-9]*\s*[0-9]*\s*$lookahead\s*NA" log_${impl}_${app} | awk '{print $(NF-2);}' | sort -n | awk '{if (NR>1) print $NF}' > temp_${impl}
                            CUR_RESULT="$CUR_RESULT, `awk 'BEGIN{s=0;}{s=s+$1;}END{print s/NR;}' temp_${impl}`,`awk '{delta = $1 - avg; avg += delta / NR; mean2 += delta * ($1 - avg); } END { print sqrt(mean2 / (NR-1));}' temp_${impl}`"
                        fi
                    done

                    echo "$app, $prec, $size, $size, $lookahead $CUR_RESULT" #`grep "$prec.*$size .* $num" log_hclib_${app} | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==5) med=$NF; else if(NR==10) max=$NF;} END {print min,med,max}'`"
#        echo "$size, $size, $num, `grep "$size.* $num" $1 | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==1) min=$NF; else if(NR==5) med=$NF; else if(NR==10) max=$NF;} END {print min,med,max}'`"
#    grep "$size.* 14" $1 | awk '{print $(NF-4);}' | sort -n | awk '{if (NR==3) print $NF}'
                done
            done
        done
    done
fi
