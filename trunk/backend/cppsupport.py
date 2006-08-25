#   			Settings.py - Copyright Daniel Benamy, Natan Zohar
# 
# Licensed under the GPLv2


from languagesupport import LanguageSupport


class CppSupport (LanguageSupport):
    def get_supported_extensions(self):
        return ['cpp', 'cxx']
