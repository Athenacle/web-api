#ifndef DECL_H_
#define DECL_H_

#define CONTAIN_GET(json, key, object)        \
    do {                                      \
        if (json.contains(#key)) {            \
            json.at(#key).get_to(object.key); \
        }                                     \
    } while (false);



// conf.h
struct ConfApp;
struct ConfObject;

#endif