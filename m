make xz
cp ../csl.tar.xz ../csl.$1.tar.xz
cp ../csl.$1.tar.xz /home/backup/csl.$1.tar.xz
mv ../csl.$1.tar.xz /home/backup/archive/csl.$1.tar.xz
ls -al /home/backup/csl.$1.tar.xz
ls -al /home/backup/archive/csl.$1.tar.xz
