# Git 导入来源与精确获取

`p75-jis-0812` 是在现有 KIBU fork 中新建的独立快照分支，初始提交没有父提交；它记录公开整理版本的导入时间，不冒称历史生产 commit。既有 master 和 kibu 分支及其历史保留。固定发布标签为 `p75-jis-0812-r1`。

输入为已接受的0812对应源码及发布资料。9月23日候选ZIP SHA-256 为 `0362C3BE9EE3FB8ECB40E9F0A6A4199D866D6D2687C53DD955C17A65A4B12C13`；吸收用户实测补充后的9月24日候选为 `30992703061754763E61C5D5994D385DB2618B3AEECD93463B0684502C922747`。公开整理新增固定入口、此说明和构建CI工具；没有修改固件源码、BIN、JSON或依赖字节。

原快照的9个失效 `.git` 指针按已有资料排除；所有随包源码依赖按普通文件纳入，不恢复为需要私有仓库的 gitlink。包内 `qmk_firmware/.gitmodules` 保留历史原文，不能用于替换这些固定依赖。完整根目录布局与ZIP一致，源码在 `qmk_firmware/`，构建入口在 `tools/`。

## 精确文件字节

源码保留原 `.gitattributes` 和 `.gitignore`。发布导入时用本地 `.git/info/attributes` 关闭换行、过滤器和ident转换，并按发布清单显式纳入文件，以免忽略规则漏掉依赖、原件或缓存文件。该本地Git配置不加入公开源码，也不修改全局设置。

普通 clone 的工作文件可能受原有属性规则影响；需要与文件清单逐字节核对时，可直接下载Release源码ZIP，或者在Bash中对一个全新目录执行：

```bash
git clone --no-checkout --branch p75-jis-0812-r1 https://github.com/kibu-keyboards/qmk_firmware.git p75-jis-0812-r1
cd p75-jis-0812-r1
printf '* -text -eol -filter -ident\n' > .git/info/attributes
git checkout --detach p75-jis-0812-r1
```

此命令不下载子模块。CI在全新checkout中设置精确字节规则，再用 `git archive HEAD` 生成临时ZIP并解压覆盖该全新工作区，避免索引缓存跳过已有文件、保留首次检出时的换行转换。`metadata/release-manifest.csv`记录除自身以外所有发布文件的大小和SHA-256，清单自身由Git提交及ZIP哈希固定。

来源、许可、已知问题和实机覆盖分别见同目录对应说明；Git导入不改变这些结论。
