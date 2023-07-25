## I2C_MANAGER
ADXL345 Component has a dependency on I2C_MANAGER  
https://github.com/ropg/i2c_manager  

- Easy to use thread-safe i2c library.  
- Doesn't compile out-of-the-box when used with ESP-IDF v5.0.x/v5.1  
 
Minor fixes that worked for me:  


```diff
index 8bb89aa..b3132ff 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -9,4 +9,7 @@ endif()
 idf_component_register(
     SRCS ${SOURCES}
     INCLUDE_DIRS ${INCLUDES}
+    REQUIRES "driver"
 )
+# Compile errors ESP_LOG_
+target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-format)
diff --git a/i2c_manager/i2c_manager.c b/i2c_manager/i2c_manager.c
index d6badfe..38a1f05 100644
--- a/i2c_manager/i2c_manager.c
+++ b/i2c_manager/i2c_manager.c
@@ -58,7 +58,7 @@ static SemaphoreHandle_t* I2C_FN(_mutex) = &I2C_FN(_local_mutex)[0];

 static const uint8_t ACK_CHECK_EN = 1;

-#if defined (I2C_NUM_0) && defined (CONFIG_I2C_MANAGER_0_ENABLED)
+#if  (I2C_NUM_0 == 0) && defined (CONFIG_I2C_MANAGER_0_ENABLED)
 	#define I2C_ZERO 					I2C_NUM_0
 	#if defined (CONFIG_I2C_MANAGER_0_PULLUPS)
 		#define I2C_MANAGER_0_PULLUPS 	true
@@ -71,7 +71,7 @@ static const uint8_t ACK_CHECK_EN = 1;
 #endif


-#if defined (I2C_NUM_1) && defined (CONFIG_I2C_MANAGER_1_ENABLED)
+#if (I2C_NUM_1 == 1) && defined (CONFIG_I2C_MANAGER_1_ENABLED)
 	#define I2C_ONE 					I2C_NUM_1
 	#if defined (CONFIG_I2C_MANAGER_1_PULLUPS)
 		#define I2C_MANAGER_1_PULLUPS 	true
```
