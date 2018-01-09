# 嵌入式 GUI 库----uGFX
### μGFX 是一款用于显示器和触摸屏的轻量级嵌入式库，提供构建全功能嵌入式GUI所需的一切。该库非常小并且速度很快，因为每个未使用的功能都被禁用并且不会链接到已完成的二进制文件中。具有以下优点：
- 轻量级
- 模块化
- 便携式
- 完全开源
- 开发活跃

### ugfx_driver 文件结构介绍
1. /adapter 文件夹下包含了屏幕驱动适配相关功能代码
2. /framebuffer 使用缓冲区相关的驱动代码
3. /include 使用 uGFX 需要包含的头文件
4. /lcd_raw uGFX 库使用的 LCD 驱动函数实现
5. /touch_screen uGFX 库使用的 Touch 驱动函数实现

### 在 IoT Solution 中使用 μGFX 库
本节将介绍如何在 IoT Solution 中使用 μGFX 库.
1. 在工程中包含头文件 ‘iot_ugfx.h’
2. 在 ‘menuconfig’ 中使能 uGFX GUI enable 和 PARAM_FUNC_ENABLE（‘IoT Solution settings'->'IoT Components Management'->'HMI components'->'uGFX GUI enable' 和 ‘IoT Solution settings'->'IoT Components Management'->‘General functions’->'PARAM_FUNC_ENABLE'）
3. 在 uGFX settings 中配置相关的屏幕信息（’IoT Solution settings'->'IoT Components Management'->'HMI components'->'uGFX GUI enable'->'uGFX settings'）
之后,即可根据工程需要使用 uGFX 库所提供的相关功能, uGFX 库的使用可以参考[uGFX官方文档](https://wiki.ugfx.io/index.php/Main_Page).

### 屏幕驱动适配
本节将介绍如何进行屏幕驱动适配,在使用自己的驱动代码时,我们需要做:
1. 在 /adapter 文件夹中将 ‘lcd_adapter’ 文件里的函数修根据使用屏幕的驱动代码进行相应修改；如果有使用 Touch 也需要对 ’touch_adater' 文件里的函数进行相应的修改；
2. 在 'make menuconfig'->'IoT Solution settings'->'IoT Components Management'->'HMI components'->'uGFX GUI enable'->'uGFX settings' 中配置屏幕相关信息（宽、高、引脚、显示方式、自动刷新等信息）
