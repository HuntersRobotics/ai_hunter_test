# Add Software Sources and Install Dependency Libraries

## Add Software Sources and Update Cache

```bash
cat << 'EOF' | sudo tee /etc/apt/sources.list.d/kaylordut.list
deb [signed-by=/etc/apt/keyrings/kaylor-keyring.gpg] http://apt.kaylordut.cn/kaylordut/ kaylordut main
EOF
cat << 'EOF' | sudo tee /etc/apt/preferences.d/kaylordut
Package: *
Pin: release o=kaylordut kaylordut,a=kaylordut,n=kaylordut,l=kaylordut kaylordut,c=main,b=arm64
Pin-Priority: 1099
EOF
sudo mkdir /etc/apt/keyrings -pv
sudo wget -O /etc/apt/keyrings/kaylor-keyring.gpg http://apt.kaylordut.cn/kaylor-keyring.gpg
sudo apt update
```

## Install Kaylordut Library
```bash
sudo apt install -y kaylordut-dev
```
> My private library source can be found via the [link](https://github.com/kaylorchen/kaylordut).