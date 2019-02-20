cd ../android

#rm -rf sdk/build/intermediates/bundles/debug/jni/*

#./gradlew clean assembleDebug --info

./gradlew assembleDebug --info



# cp weexcore
cp sdk/build/intermediates/bundles/debug/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/bundles/debug/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/bundles/debug/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so


#cp jss
cp sdk/build/intermediates/bundles/debug/jni/armeabi/libweexjss.so sdk/libs/armeabi/libweexjss.so
cp sdk/build/intermediates/bundles/debug/jni/armeabi-v7a/libweexjss.so sdk/libs/armeabi-v7a/libweexjss.so
cp sdk/build/intermediates/bundles/debug/jni/x86/libweexjss.so sdk/libs/x86/libweexjss.so



# cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
# cp sdk/build/intermediates/cmake/debug/obj/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
# cp sdk/build/intermediates/cmake/debug/obj/x86/libweexcore.so sdk/libs/x86/libweexcore.so


# adb

cd playground/app/build/outputs/apk/

adb uninstall com.alibaba.weex

adb install playground-debug.apk
