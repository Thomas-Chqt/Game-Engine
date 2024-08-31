/*
 * ---------------------------------------------------
 * Application.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 14:49:37
 * ---------------------------------------------------
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

class Application
{
public:
    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    inline bool shouldTerminate() { return m_shouldTerminate; }

    virtual void onSetup() {}
    virtual void onUpdate() {}

    virtual ~Application() = default;

protected:
    Application() = default;

    bool m_shouldTerminate = true;
    
public:
    Application& operator = (const Application&) = delete;
    Application& operator = (Application&&)      = delete;
};

#endif // APPLICATION_HPP