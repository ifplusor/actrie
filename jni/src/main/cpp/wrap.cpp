#include <stdlib.h>
#include <string.h>

#include <psn_ifplusor_actrie_Context.h>
#include <psn_ifplusor_actrie_Matcher.h>

#include <matcher.h>
#include <utf8ctx.h>

/*
 * Class:     psn_ifplusor_actrie_Matcher
 * Method:    ConstructByFile
 * Signature: (Ljava/lang/String;ZZZZ)J
 */
JNIEXPORT jlong JNICALL Java_psn_ifplusor_actrie_Matcher_ConstructByFile(JNIEnv* env,
                                                                         jclass clazz,
                                                                         jstring filepath,
                                                                         jboolean all_as_plain,
                                                                         jboolean ignore_bad_pattern,
                                                                         jboolean bad_as_plain,
                                                                         jboolean deduplicate_extra) {
  if (filepath == NULL) {
    return 0;
  }

  const char* utf = env->GetStringUTFChars(filepath, JNI_FALSE);
  if (utf == NULL) return 0;
  // jsize len = env->GetStringUTFLength(filepath);

  matcher_t matcher = matcher_construct_by_file(utf, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra);

  env->ReleaseStringUTFChars(filepath, utf);

  return (jlong)matcher;
}

/*
 * Class:     psn_ifplusor_actrie_Matcher
 * Method:    ConstructByString
 * Signature: (Ljava/lang/String;ZZZZ)J
 */
JNIEXPORT jlong JNICALL Java_psn_ifplusor_actrie_Matcher_ConstructByString(JNIEnv* env,
                                                                           jclass clazz,
                                                                           jstring keywords,
                                                                           jboolean all_as_plain,
                                                                           jboolean ignore_bad_pattern,
                                                                           jboolean bad_as_plain,
                                                                           jboolean deduplicate_extra) {
  if (keywords == NULL) {
    return 0;
  }

  const char* utf = env->GetStringUTFChars(keywords, JNI_FALSE);
  if (utf == NULL) return 0;
  jsize len = env->GetStringUTFLength(keywords);

  strlen_s vocab = {.ptr = (char*)utf, .len = (size_t)len};
  matcher_t matcher =
      matcher_construct_by_string(&vocab, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra);

  env->ReleaseStringUTFChars(keywords, utf);

  return (jlong)matcher;
}

/*
 * Class:     psn_ifplusor_actrie_Matcher
 * Method:    Destruct
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_psn_ifplusor_actrie_Matcher_Destruct(JNIEnv* env, jclass clazz, jlong matcher) {
  matcher_destruct((matcher_t)matcher);
  return JNI_TRUE;
}

/*
 * Class:     psn_ifplusor_actrie_Context
 * Method:    AllocContext
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_psn_ifplusor_actrie_Context_AllocContext(JNIEnv* env, jclass clazz, jlong matcher) {
  if (matcher == 0) {
    return 0;
  }

  return (jlong)utf8ctx_alloc_context((matcher_t)matcher);
}

/*
 * Class:     psn_ifplusor_actrie_Context
 * Method:    FreeContext
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_psn_ifplusor_actrie_Context_FreeContext(JNIEnv* env, jclass clazz, jlong context) {
  utf8ctx_free_context((utf8ctx_t)context);
  return JNI_TRUE;
}

/*
 * Class:     psn_ifplusor_actrie_Context
 * Method:    ResetContext
 * Signature: (JLjava/lang/String;Z)Z
 */
JNIEXPORT jboolean JNICALL Java_psn_ifplusor_actrie_Context_ResetContext(JNIEnv* env,
                                                                         jclass clazz,
                                                                         jlong context,
                                                                         jstring content,
                                                                         jboolean return_byte_pos) {
  if (context == 0 || content == NULL) {
    return JNI_FALSE;
  }

  const char* utf = env->GetStringUTFChars(content, JNI_FALSE);
  if (utf == NULL) return JNI_FALSE;
  jsize len = env->GetStringUTFLength(content);

  jboolean code = utf8ctx_reset_context((utf8ctx_t)context, (char*)utf, len, return_byte_pos) ? JNI_TRUE : JNI_FALSE;

  env->ReleaseStringUTFChars(content, utf);

  return code;
}

#define BUILDWORD_METHOD_SIG "(Ljava/lang/String;JJLjava/lang/String;)Lpsn/ifplusor/actrie/Word;"
#define OUTOFMEMORYERROR_CLASS_PATH "java/lang/OutOfMemoryError"

static inline jstring get_matched_keyword(JNIEnv* env, word_t matched_word) {
  char* s = (char*)malloc(matched_word->keyword.len + 1);
  if (s == NULL) {
    jclass OutOfMemoryError = env->FindClass(OUTOFMEMORYERROR_CLASS_PATH);
    if (OutOfMemoryError == NULL) {
      abort();
    }
    env->ThrowNew(OutOfMemoryError, "actrie: can not alloc memory for keyword");
    return NULL;
  }
  strncpy(s, matched_word->keyword.ptr, matched_word->keyword.len);
  s[matched_word->keyword.len] = '\0';
  jstring keyword = env->NewStringUTF(s);
  free(s);
  return keyword;
}

static inline jobject build_matched_output(JNIEnv* env, jclass clazz, utf8ctx_t utf8ctx, word_t matched_word) {
  jmethodID buildWord = env->GetStaticMethodID(clazz, "buildWord", BUILDWORD_METHOD_SIG);
  if (buildWord == NULL) return NULL;
  jstring keyword = get_matched_keyword(env, matched_word);
  if (keyword == NULL) return NULL;
  jstring extra = env->NewStringUTF(matched_word->extra.ptr);
  if (extra == NULL) return NULL;
  jobject word = env->CallStaticObjectMethod(clazz, buildWord, keyword, (jlong)matched_word->pos.so,
                                             (jlong)matched_word->pos.eo, extra);
  return word;
}

typedef word_t (*utf8ctx_next_f)(utf8ctx_t utf8ctx);

static jobject next(JNIEnv* env, jclass clazz, jlong context, utf8ctx_next_f utf8ctx_next_func) {
  if (context == 0) {
    return NULL;
  }

  word_t matched_word = utf8ctx_next_func((utf8ctx_t)context);
  if (matched_word != NULL) {
    return build_matched_output(env, clazz, (utf8ctx_t)context, matched_word);
  }

  return NULL;
}

/*
 * Class:     psn_ifplusor_actrie_Context
 * Method:    Next
 * Signature: (J)Lpsn/ifplusor/actrie/Word;
 */
JNIEXPORT jobject JNICALL Java_psn_ifplusor_actrie_Context_Next(JNIEnv* env, jclass clazz, jlong context) {
  return next(env, clazz, context, utf8ctx_next);
}
