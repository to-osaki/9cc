#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    clang -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit1
    fi
}

assert 0 '0;'
assert 42 'return 42;'
assert 21 '5+20-4;'
assert 30 '15-0+15;'
assert 41 ' 12 + 34 - 5 ;'
assert 6 '3    +5-2;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 4 ' (5 + (( 3)  ))*2/4;'
assert 3 '+3;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 6 'a = 3;3+a;'
assert 18 'x = 3;va=-16;return -(x*(10+va));'
assert 6 'fuga = 1;hoge = 2+3;return fuga+hoge;'
assert 3 'return 3;return 5;return 10;'

assert 1 'goto hoge; return 0; hoge: return 1;'
assert 20 'a = 2*(3+2); goto fuga; return a; fuga: return a * 2;'
assert 201 'b = 100; a = 101;c=1; goto hoge; return c;hoge: return a+b;'

echo OK