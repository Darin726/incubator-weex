cd ../android/sdk

#rm -rf sdk/build/*

../gradlew assembleDebug --info

cd ..


# cp weexcore
cp sdk/build/intermediates/bundles/debug/jni/armeabi/libweexcore.so sdk/libs/armeabi/libweexcore.so
cp sdk/build/intermediates/bundles/debug/jni/armeabi-v7a/libweexcore.so sdk/libs/armeabi-v7a/libweexcore.so
cp sdk/build/intermediates/bundles/debug/jni/x86/libweexcore.so sdk/libs/x86/libweexcore.so


#cp jss
cp sdk/build/intermediates/bundles/debug/jni/armeabi/libweexjss.so sdk/libs/armeabi/libweexjss.so
cp sdk/build/intermediates/bundles/debug/jni/armeabi-v7a/libweexjss.so sdk/libs/armeabi-v7a/libweexjss.so
cp sdk/build/intermediates/bundles/debug/jni/x86/libweexjss.so sdk/libs/x86/libweexjss.so




# back up obj

rm -rf ~/Desktop/weex_so_armeabi_debug
mkdir ~/Desktop/weex_so_armeabi_debug


cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexcore.so ~/Desktop/weex_so_armeabi_debug/
cp sdk/build/intermediates/cmake/debug/obj/armeabi/libweexjss.so ~/Desktop/weex_so_armeabi_debug/