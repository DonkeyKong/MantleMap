#pragma once

#include <memory>

class UsbButton
{
public:
    UsbButton();
    ~UsbButton();
    bool pressed();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

#endif

