cvs_bin = cvs.exe
cvs_server = 172.16.66.111

# ============ 用户编辑部分 ============

## 用户帐号
cvs_user = gaokp

## 源数据库
cvs_depot = Audio

## 源模块
cvs_src_mod = Codec/Dec7222-x86

## 目标模块
cvs_dst_mod = Audio/x86/Dec7222

## 目标 rar 文件
cvs_dst_fname = Dec7222.rar

# ========== 用户编辑部分结束 ==========

SRCROOT=:pserver:$(cvs_user)@$(cvs_server):/$(cvs_depot)
DSTROOT=:pserver:$(cvs_user)@$(cvs_server):/Product

zip_cmd:=rar

opdir:=cvs_op

rcs_tag=PRD_$(cvs_user)_$(rcs_ver)_$(rcs_date)

comma:=,
period:=.
shortline:=-
empty:=
space:=$(empty) $(empty)

all: ver_tag

# 第一次 login 要保持 44, 45 行, login 过后可以用 '#' 注释
# === checkout all files ===
co_files:
	@$(shell mkdir $(opdir))
#	$(cvs_bin) -d $(SRCROOT) login
#	$(cvs_bin) -d $(DSTROOT) login	
	cd $(opdir) &&  \
	 $(cvs_bin) -d $(SRCROOT) checkout $(cvs_src_mod) && \
	 $(cvs_bin) -d $(DSTROOT) checkout $(cvs_dst_mod) && \
	 cd $(cvs_dst_mod) && rm -f $(cvs_dst_fname)
	
# === zip source moduel files ===
zip_src_files: co_files
	cd $(opdir)/$(cvs_src_mod) && \
	 $(zip_cmd) a $(cvs_dst_fname) -r src build doc test 
	mv $(opdir)/$(cvs_src_mod)/$(cvs_dst_fname) $(opdir)/$(cvs_dst_mod)/$(cvs_dst_fname)

# 新加入文件请去掉第 60, 61 行前面的 '#'，除此以外就不用
# === commit new zip file ===
renew_dst_files: zip_src_files
#	cd $(opdir)/$(cvs_dst_mod) && \
#	 $(cvs_bin) -d $(DSTROOT) add -kb $(cvs_dst_fname)
	cd $(opdir)/$(cvs_dst_mod) && \
	 $(cvs_bin) -d $(DSTROOT) commit -m "" -f $(cvs_dst_fname)

# === generate tag ===
ver_tag: renew_dst_files
#	$(cvs_bin) -d $(DSTROOT) history -c -l -f $(cvs_dst_fname)
	export rcs_ver=$(subst $(period),$(shortline),$(word 6, \
	 $(shell $(cvs_bin) -d $(DSTROOT) history -c -l -f $(cvs_dst_fname)))) && \
	 export rcs_date=$(subst $(shortline),$(empty),$(word 2, \
	 $(shell $(cvs_bin) -d $(DSTROOT) history -c -l -f $(cvs_dst_fname)))) && \
	 make -f rel.mak tag_src_files

# === tag source files ===
tag_src_files:
	cd $(opdir)/$(cvs_src_mod) && $(cvs_bin) -d $(SRCROOT) tag -c -F -R $(rcs_tag)
	rm -rf $(opdir)
			
clean:
	rm -rf $(opdir)
