- 👋 Hi, I’m @lc6261
- 👀 I’m interested in ...
- 🌱 I’m currently learning ...
- 💞️ I’m looking to collaborate on ...
- 📫 How to reach me ...
- ---------------------------gihub----------------------------------------------
- GitHub Desktop的安装和使用「2020.7.8」
- 下载:https://desktop.github.com/
- 原文链接：https://blog.csdn.net/JCtry/article/details/107198217
- ---------------------------clash----------------------------------------------
- clash.net
- 下载
- https://github.com/Fndroid/clash_for_windows_pkg/releases
- 汉化
- https://github.com/BoyceLig/Clash_Chinese_Patch/releases
- 订阅转换：https://bit.ly/2HBq9EY
- ----------------------------vps相关---------------------------------------------
- 查看路由速率https://tools.ipip.net/traceroute.php
- 免费域名申请网站https://www.freenom.com/ 
- 域名解析网站：https://dash.cloudflare.com/
- Centos安装宝塔面板脚本
- yum install -y wget && wget -O install.sh http://download.bt.cn/install/install_6.0.sh && sh install.sh
- 安装科学上网命令
- bash <(curl -sL https://s.hijk.art/v2ray.sh)
- -------------------------------------------------------------------------
BBR是Google提出的提升TCP效率的算法，在丢包率较高的网络上，可大幅度提升通信效率。BBR算法已在Linux上实现为内核补丁，不过比较旧的版本没有应用此补丁，新版本即使有也可能默认没有开启。本文介绍如何确认VPS上是否开启了BBR，以及如果没有开启，如何开启。

# 第一项检查：
root@topvps:~# sysctl net.ipv4.tcp_available_congestion_control | grep bbr

# 若已开启bbr，结果通常为以下两种：
net.ipv4.tcp_available_congestion_control = bbr cubic reno
net.ipv4.tcp_available_congestion_control = reno cubic bbr

# 第二项检查：
root@topvps:~# sysctl net.ipv4.tcp_congestion_control | grep bbr
# 若已开启bbr，结果如下：
net.ipv4.tcp_congestion_control = bbr

# 第三项检查：
root@topvps:~# sysctl net.core.default_qdisc | grep fq
# 若已开启bbr，结果如下：
net.core.default_qdisc = fq

# 第四项检查：
root@topvps:~# lsmod | grep bbr
# 若已开启bbr，结果可能如下。并不是所有的 VPS 都会有此返回值，若没有也属正常。
tcp_bbr                20480  2


安装wget
yum -y install wget

安装bbr
wget --no-check-certificate https://github.com/teddysun/across/raw/master/bbr.sh && chmod +x bbr.sh && ./bbr.sh


- ----------------------------转发---------------------------------------------
- #开启系统路由模式功能
- echo net.ipv4.ip_forward=1>>/etc/sysctl.conf

- #运行这个命令会输出上面添加的那一行信息，意思是使内核修改生效
- sysctl -p
- #开启firewalld
- systemctl start firewalld
- #开启4650端口监听tcp请求
- firewall-cmd --zone=public --add-port=4567/tcp --permanent
- #设置IP地址伪装
- firewall-cmd --add-masquerade --permanent
- #设置端口映射
- firewall-cmd --add-forward-port=port=4567:proto=tcp:toaddr=172.65.239.73:toport=14444 --permanent
- firewall-cmd --add-masquerade --permanent
- #重启firewall
- firewall-cmd --reload
