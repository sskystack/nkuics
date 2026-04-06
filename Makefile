-include nemu/Makefile.git

default:
	@echo "Please run 'make' under any subprojects to compile."
 
clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	# 方案一：使用减号 '-' 告诉 make 忽略该命令的任何报错
	-tar cj . > /tmp/$(STU_ID).tar.bz2
	# 或者方案二（更优雅）：如果 tar 返回 1（文件变动警告），也认为它成功
	# tar cj . > /tmp/$(STU_ID).tar.bz2 || [ $$? -eq 1 ]
	mv /tmp/$(STU_ID).tar.bz2 .
	@echo "打包成功！已处理文件变动警告。"

.PHONY: default clean submit
