MUPP_VERSION_NAME=0.26.4.21-foldDevice-SNAPSHOT
cd android
sed  "/playground/d;/commons/d" settings.gradle
sed  '/args/d' settings.gradle

ASanFlag="false"
if [ "$1" = "enableASan" ];then
ASanFlag="true"
fi

echo "--> # build normal"

./gradlew weex_sdk:assembleRelease -PweexVersion=${MUPP_VERSION_NAME} -PbuildRuntimeApi=false -Pjsc_url="http://mtl.alibaba-inc.com/oss/mupp/12068284/android/sdk/build/outputs/aar/weex_sdk-release.aar" -Paar_name="weex_sdk" -PjsInterpolatorName="JavaScriptCore" -PremoveSharedLib="true" -PsupportArmeabi="false" -PdisableCov="true" -PenableASan=${ASanFlag} -Pexternal_script="http://gitlab.alibaba-inc.com/android-build-system/buildscript/raw/master/mtl-publish-2.3.gradle http://gitlab.alibaba-inc.com/EMAS-Android/android-buildscript/raw/master/add-emas-publish-repository.gradle" -Pexternal_repositories="http://mvnrepo.alibaba-inc.com/mvn/repository"  -DEMASPUBLISH=false --info

echo "--> # 3. publish"

./gradlew weex_sdk:assembleRelease weex_sdk:publish -PweexVersion=${MUPP_VERSION_NAME} -PbuildRuntimeApi=false -Pjsc_url="http://mtl.alibaba-inc.com/oss/mupp/12068284/android/sdk/build/outputs/aar/weex_sdk-release.aar" -Paar_name="weex_sdk" -PjsInterpolatorName="JavaScriptCore" -PremoveSharedLib="true" -PsupportArmeabi="false" -PdisableCov="true" -PenableASan=${ASanFlag} -Pexternal_script="http://gitlab.alibaba-inc.com/android-build-system/buildscript/raw/master/mtl-publish-2.3.gradle http://gitlab.alibaba-inc.com/EMAS-Android/android-buildscript/raw/master/add-emas-publish-repository.gradle" -Pexternal_repositories="http://mvnrepo.alibaba-inc.com/mvn/repository" -DEMASPUBLISH=false --info