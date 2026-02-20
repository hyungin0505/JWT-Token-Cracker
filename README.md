# JWT-Token-Cracker

```bash
brew install openssl@3
brew install libomp
```

MAC 환경에서 재현  

```bash
clang -O3 -Xpreprocessor -fopenmp \  -I$(brew --prefix libomp)/include \  -L$(brew --prefix libomp)/lib -lomp \
  -I$(brew --prefix openssl@3)/include \
  -L$(brew --prefix openssl@3)/lib -lcrypto \
  crack.c -o crack
```

```bash
./crack <wordlist.file> <JWT.TOKEN>
```
