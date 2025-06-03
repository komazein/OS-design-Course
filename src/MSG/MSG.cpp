#include"MSG.h"
// void MSGSTR::trans(MSGTYPE type)
// {
//     if(type==ERROR)
//         msgstr.push_back({"[ERROR]:",Color::Red});
//     if(type==WARN)
//         msgstr.push_back({"[WARN]:",Color::Yellow});
//     if(type==DEBUG)
//         msgstr.push_back({"[DEBUG]:",Color::Magenta});
//     if(type==INFO)
//         msgstr.push_back({"[INFO]:",Color::Magenta});
//     return;
// }
// // template<typename T,typename... Args>
// // void MSGSTR::msglog(MSGTYPE msg, string base,T& first, Args... args)
// // {
// //     trans(msg);
// //     size_t pos = base.find("{}");
// //     base.replace(pos, 2,to_string(forward<T>(first)));
// //     return msglog(base, forward<Args>(args)...);
// // }
// // void MSGSTR::msglog(MSGTYPE msg, string base)
// // {
// //     trans(msg);
// //     msgstr.push_back({base,Color::White});
// //     cout<<"{"<<endl;
// //     cout<<base<<endl;
// //     return;
// // }
// // // template<typename T,typename... Args>
// // // void MSGSTR::msglog(string base,T& first, Args... args)
// // // {
// // //     size_t pos = base.find("{}");
// // //     base.replace(pos, 2,to_string(forward<T>(first)));
// // //     return msglog(base, forward<Args>(args)...);
// // // }
// // void MSGSTR::msglog(string base)
// // {
// //     msgstr.push_back({base,Color::White});
// //     cout<<"{"<<endl;
// //     cout<<base<<endl;
// //     return;
// // }
// vector<pair<string,Color>> MSGSTR::RETstr()
// {
//     return msgstr;
// }
// void MSGSTR::clean()
// {
//     msgstr.clear();
//     return;
// }
// void MSGSTR::msgdirentry(string filename,TYPE filetype)
// {
//     if(filetype==DIR)
//         msgstr.push_back({filename,Color::Blue});
//     else
//         msgstr.push_back({filename,Color::Green});
//     return;
// }