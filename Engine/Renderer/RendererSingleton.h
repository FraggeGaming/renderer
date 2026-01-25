#include <mutex>

class RendererSingleton{

private:
    static std::mutex m_mutex;
    static RendererSingleton* m_renderer;

protected:
    RendererSingleton(){};
    ~RendererSingleton() {}

public:
    RendererSingleton(RendererSingleton* other) = delete;
    void operator=(const RendererSingleton&) = delete;
    static RendererSingleton* GetInstance();
};

RendererSingleton* RendererSingleton::m_renderer{nullptr};
std::mutex RendererSingleton::m_mutex;

RendererSingleton *RendererSingleton::GetInstance()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_renderer==nullptr){
        m_renderer = new RendererSingleton();
    }
    return m_renderer;
}

