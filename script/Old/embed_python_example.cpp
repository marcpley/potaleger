// Exemple C++ minimal d'initialisation d'un Python embarqué
// - adapte les chemins et conversion wchar selon plateforme
// - tu peux remplacer Py_SetPath/Py_SetPythonHome par l'API PyConfig (Python >=3.8)

#include <Python.h>
#include <string>
#include <vector>

// Déclare la fonction d'init du module 'app' (exposé via pybind11 ou module CPython)
// Si tu utilises pybind11, la fonction générée s'appelera PyInit_app()
extern "C" PyObject* PyInit_app();

static std::wstring to_wstring(const std::string &s) {
    return std::wstring(s.begin(), s.end()); // simplifié ; sur Windows utilises MultiByteToWideChar si nécessaire
}

int init_embedded_python(const std::string &embed_root)
{
    // embed_root : chemin vers le dossier python embarqué (ex: "<appdir>/python")
    // Construction d'un PYTHONPATH (séparateur ';' sur Windows, ':' sur Unix)
#ifdef _WIN32
    const wchar_t sep = L';';
#else
    const wchar_t sep = L':';
#endif

    // Exemple de chemins (adapter)
    std::wstring wroot = to_wstring(embed_root);
#ifdef _WIN32
    std::wstring python_home = wroot; // contient DLLs, Lib, etc.
    std::wstring python_path = wroot + L"\\Lib" + std::wstring(1, sep)
                              + wroot + L"\\DLLs" + std::wstring(1, sep)
                              + wroot + L"\\Lib\\site-packages";
#else
    std::wstring python_home = wroot;
    std::wstring python_path = wroot + L"/lib/python3.11" + std::wstring(1, sep)
                              + wroot + L"/lib/python3.11/site-packages";
#endif

    // Exposer le module app (doit être compilé et lié à l'exe)
    if (PyImport_AppendInittab("app", &PyInit_app) == -1) {
        // erreur
        return -1;
    }

    // Définir PythonHome/Path avant Py_Initialize
    Py_SetPythonHome(const_cast<wchar_t*>(python_home.c_str()));
    Py_SetPath(const_cast<wchar_t*>(python_path.c_str()));

    // Initialiser
    Py_Initialize();
    if (!Py_IsInitialized()) return -1;

    // (option) initialiser sys.argv si besoin
    wchar_t *program = Py_DecodeLocale("myapp", nullptr);
    PySys_SetArgv(1, &program);

    // Importer ton module app pour vérifier
    PyObject* pmod = PyImport_ImportModule("app");
    if (!pmod) {
        PyErr_Print();
        return -1;
    }
    Py_DECREF(pmod);

    return 0;
}
