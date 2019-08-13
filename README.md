# pkwalker
某を自動で歩かせるやつ

# compile
```
gcc pkwalker.c  -o pkwalker -l imobiledevice
```

# usage
## start walking
```
./pkwalker coordinate.dat
```

## stop walking
```
./pkwalker -s
```

# coordinate.dat
```
<緯度> <経度>
```
## example
https://github.com/Goyotan/pkwalker/blob/master/coordinate.dat

# Reference
- idevicelocation
- libimobiledevice API
