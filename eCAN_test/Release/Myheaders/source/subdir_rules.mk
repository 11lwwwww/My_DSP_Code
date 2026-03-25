################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Myheaders/source/%.obj: ../Myheaders/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/CCS/CCS12.8.1/app/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 -O2 --include_path="D:/CCS/CCS12.8.1/workspace_v12.8.1/eCAN_test" --include_path="D:/CCS/CCS12.8.1/app/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="Myheaders/source/$(basename $(<F)).d_raw" --obj_directory="Myheaders/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


