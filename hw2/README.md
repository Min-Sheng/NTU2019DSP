# Usage

1. Download HTK tools package(htk341_debian_x86_64.tar.gz) as well as this all repository.

2. Unzip this the HTK tools package to whatever place, just like

```bash
$ tar -zxvf htk341_debian_x86_64.tar.gz
```

3. Set the path of the unzipped HTK tools directory in "set_htk_path.sh", like

```bash
$ PATH=$PATH:“~/XXXX/XXXX”
```

4. In case shell script is not permitted to run, change the permission

```bash
$ chmod 744 XXXX.sh
```

5. Execute the scripts bellow:
- 01_run_HCopy.sh
- 02_run_HCompV.sh
- 03_training.sh
- 04_testing.sh

## Note
Every time you modified **any parameter or file**, run 00_clean_all.sh to remove all the files that
were produced before, and restart all the procedures. If
not, the new settings will be performed on the previous
files, and hence you will be not able to analyze the new
results. (Of course, you should record your current results
before starting the next experiment.)

