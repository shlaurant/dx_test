#include "debug.h"
#include <comdef.h>

using namespace Microsoft::WRL;

DxException::DxException(HRESULT hr, const std::wstring &functionName,
                         const std::wstring &filename, int lineNumber) :
        ErrorCode(hr),
        FunctionName(functionName),
        Filename(filename),
        LineNumber(lineNumber) {
}

std::wstring DxException::ToString() const {
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = (wchar_t *) err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " +
           std::to_wstring(LineNumber) + L"; error: " + msg;
}