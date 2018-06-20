# Test
NotePad++에 Plug-In을 Korean Charset Check하는 것을 만들어 올리고, Git을 처음 작성해 보고 있다.

- EUC-KR (행망코드)
- CP949 (MS949, CodePage 949)
- UTF-8

위 세가지 Charset을 혼용하는 과정에서 한글 문제가 걸린다.

외국 Site에서 소스화일을 얻게 되면 주석부분에서 일부가 문제가 되는 경우가 발생한다.

예> Visual C++에서는  warning C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다. 데이터가 손실되지 않게 하려면 해당 파일을 유니코드 형식으로 저장하십시오.

예> git에서는 충돌된 부분의 주석이 무조건 UTF-8로 추가된다. 현재문서는 EUC-KR임에도 불구한데도.

설치방법
---------------------------------------
1. [nppKoreanCharsetCheck.dll](./bin/nppKoreanCharsetCheck.dll)을 다운받는다.
2. nppKoreanCharsetCheck.dll화일을 C:\Program Files (x86)\Notepad++\plugins에 넣는다.
   (만일, x64이면 [nppKoreanCharsetCheck.dll](./bin64/nppKoreanCharsetCheck.dll)을 다운받는다.)