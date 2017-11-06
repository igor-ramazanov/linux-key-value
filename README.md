# linux-key-value
Key value persistent storage as Linux kernel module

Group project in a university. Made with Erik Ramos and Harendarczyk Bastien.

## Build
```
cd module
make
sudo insmod shared_map.ko
```

## Use
```
cd program
make
#For saving a value by a key
./test set test_key test_value
#For retrieving value by a key
./test get test_key
```

## Benchmarking
```
cd benchmark
make
./run.sh
```

## Description 
The project's description can be found in the `report` directory
