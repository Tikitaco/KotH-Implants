scp -i OB_Linux_FTP_id_rsa ./bash root@10.5.0.3:/lib/x86_64-linux-gnu/
ssh -i OB_Linux_FTP_id_rsa root@10.5.0.3 "cp -rf /lib/x86_64-linux-gnu/ /bin/bash"