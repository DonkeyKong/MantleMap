#pragma once

#include <memory>

enum class ButtonAction
{
    None,
    Tap,
    DoubleTap,
    TripleTap,
    Hold,
    Exit,
    Unsupported
};

class InputButton
{
public:
    InputButton() = default;
    virtual ~InputButton() = default;
    virtual ButtonAction PopAction() = 0;
};

#ifdef LINUX_HID_CONTROLLER_SUPPORT
class UsbButton : public InputButton
{
public:
    UsbButton();
    ~UsbButton();
    virtual ButtonAction PopAction() override;
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};
#endif
