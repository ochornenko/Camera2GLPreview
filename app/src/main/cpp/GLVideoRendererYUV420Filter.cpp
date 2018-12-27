#include "GLVideoRendererYUV420Filter.h"
#include "GLShaders.h"

GLVideoRendererYUV420Filter::GLVideoRendererYUV420Filter()
{
    m_fragmentShader.push_back(kFragmentShader);
    m_fragmentShader.push_back(kFragmentShader1);
    m_fragmentShader.push_back(kFragmentShader2);
    m_fragmentShader.push_back(kFragmentShader3);
    m_fragmentShader.push_back(kFragmentShader4);
    m_fragmentShader.push_back(kFragmentShader5);
    m_fragmentShader.push_back(kFragmentShader6);
    m_fragmentShader.push_back(kFragmentShader7);
    m_fragmentShader.push_back(kFragmentShader8);
    m_fragmentShader.push_back(kFragmentShader9);
    m_fragmentShader.push_back(kFragmentShader10);
    m_fragmentShader.push_back(kFragmentShader11);
    m_fragmentShader.push_back(kFragmentShader12);
}

GLVideoRendererYUV420Filter::~GLVideoRendererYUV420Filter()
{

}

void GLVideoRendererYUV420Filter::applyFilter(int filter)
{
    m_filter = (size_t)filter;
}

int GLVideoRendererYUV420Filter::getMaxFilter()
{
    return (int)m_fragmentShader.size();
}

void GLVideoRendererYUV420Filter::render()
{
    if (m_filter != m_prevFilter)
    {
        m_prevFilter = m_filter;

        if (m_filter >= 0 && m_filter < m_fragmentShader.size())
        {
            isProgramChanged = true;
            delete_program(m_program);
            createProgram(kVertexShader, m_fragmentShader.at(m_filter));
        }
    }

    GLVideoRendererYUV420::render();
}
