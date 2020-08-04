#!/bin/sh

assert() {
    expected="$1"
    input="$2"

    ./mycc "$input" > tmp.s
    if [ $? -ne 0 ]; then
        echo "compilation failed"
        exit 1
    fi
    cc -o tmp tmp.s
    if [ $? -ne 0 ]; then
        echo "link failed"
        exit 1
    fi
    ./tmp
    actual=$?

    if [ $actual = $expected ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected but got $actual"
        exit 1
    fi
}

## step 1
#assert 0 0
#assert 42 42
## step 2
#assert 21 "5+20-4"
## step 3
#assert 41 " 12 + 34 - 5 "
## no test for step 4
## step 5
#assert 47 '5+6*7'
#assert 15 '5*(9-6)'
#assert 4 '(3+5)/2'
## step 6
#assert 10 "-10+20"
#assert 50 "-(10*10)+150"
#assert 15 "+10+5"
#assert 150 "+(70*2)+10"
## step 7
#assert 1 "42==(4*10+2)"
#assert 0 "(20+1)==13"
#assert 1 "42!=(4*10)"
#assert 0 "(7*6)!=42"
#assert 1 "1<2"
#assert 0 "2<2"
#assert 1 "2<=2"
#assert 0 "2<=1"
#assert 1 "2>1"
#assert 0 "2>2"
#assert 1 "2>=2"
#assert 0 "1>=2"
## no test for step 8

# step 9
assert 14 "14;"
assert 28 "a=28;"
assert 15 "a=3;b=a+9;a+b;"
assert 0 "a=4/3;b=4*3;a>b;"

echo OK
