
redirfd -w 1 test5_output.txt
redirfd -a 2 test5_error.txt
pipeline {
    sort test5.input
} pipeline {
    cat b - 
}
tr A-Z a-z