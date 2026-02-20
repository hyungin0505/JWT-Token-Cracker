# JWT-Token-Cracker

```bash
brew install openssl@3
brew install libomp
```

```bash
clang -O3 -Xpreprocessor -fopenmp \
  -I$(brew --prefix libomp)/include \
  -L$(brew --prefix libomp)/lib -lomp \
  -I$(brew --prefix openssl@3)/include \
  -L$(brew --prefix openssl@3)/lib -lcrypto \
  crack.c -o crack
```

```bash
./crack <wordlist.file> <JWT.TOKEN>
```

[Wordlist Example](https://github.com/wallarm/jwt-secrets/blob/master/jwt.secrets.list)
