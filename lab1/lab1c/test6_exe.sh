
redirfd -w 1 test6_output.txt
redirfd -a 2 test6_error.txt
pipeline {
    cat test6_input
} pipeline {
    grep CS111
}
sort