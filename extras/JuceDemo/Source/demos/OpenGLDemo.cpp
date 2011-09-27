/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"

#if JUCE_OPENGL

//==============================================================================
class DemoOpenGLCanvas  : public OpenGLComponent,
                          public Timer
{
public:
    DemoOpenGLCanvas()
        : rotation (0.0f),
          delta (1.0f)
    {
        startTimer (20);

        // Just for demo purposes, let's dump a list of all the available pixel formats..
        OwnedArray <OpenGLPixelFormat> availablePixelFormats;
        OpenGLPixelFormat::getAvailablePixelFormats (this, availablePixelFormats);

        for (int i = 0; i < availablePixelFormats.size(); ++i)
        {
            const OpenGLPixelFormat* const pixFormat = availablePixelFormats[i];

            DBG (i << ": RGBA=(" << pixFormat->redBits
                   << ", " << pixFormat->greenBits
                   << ", " << pixFormat->blueBits
                   << ", " << pixFormat->alphaBits
                   << "), depth=" << pixFormat->depthBufferBits
                   << ", stencil=" << pixFormat->stencilBufferBits
                   << ", accum RGBA=(" << pixFormat->accumulationBufferRedBits
                   << ", " << pixFormat->accumulationBufferGreenBits
                   << ", " << pixFormat->accumulationBufferBlueBits
                   << ", " << pixFormat->accumulationBufferAlphaBits
                   << "), full-scene AA="
                   << (int) pixFormat->fullSceneAntiAliasingNumSamples);
        }
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create the textures needed.
    void newOpenGLContextCreated()
    {
        texture1 = createImage1();
        texture2 = createImage2();

        // (no need to call makeCurrentContextActive(), as that will have
        // been done for us before the method call).
        glDepthFunc (GL_LESS);
        glEnable (GL_DEPTH_TEST);
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_BLEND);
        glShadeModel (GL_SMOOTH);

        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void mouseDrag (const MouseEvent& e)
    {
        delta = e.getDistanceFromDragStartX() / 100.0f;
        repaint();
    }

    void renderOpenGL()
    {
        OpenGLHelpers::clear (Colours::darkgrey.withAlpha (0.0f));
        OpenGLHelpers::prepareFor2D (getWidth(), getHeight());

        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        OpenGLFrameBuffer& frameBuffer1 = dynamic_cast <OpenGLFrameBufferImage*> (texture1.getSharedImage())->frameBuffer;
        OpenGLFrameBuffer& frameBuffer2 = dynamic_cast <OpenGLFrameBufferImage*> (texture2.getSharedImage())->frameBuffer;

        frameBuffer1.draw2D (50.0f, getHeight() - 50.0f,
                             getWidth() - 50.0f, getHeight() - 50.0f,
                             getWidth() - 50.0f, 50.0f,
                             50.0f, 50.0f,
                             Colours::white.withAlpha (fabsf (::sinf (rotation / 100.0f))));

        glClear (GL_DEPTH_BUFFER_BIT);

        OpenGLHelpers::setPerspective (45.0, getWidth() / (double) getHeight(), 0.1, 100.0);

        glTranslatef (0.0f, 0.0f, -5.0f);
        glRotatef (rotation, 0.5f, 1.0f, 0.0f);

        // this draws the sides of our spinning cube..
        frameBuffer1.draw3D (-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, Colours::white);
        frameBuffer1.draw3D (-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, Colours::white);
        frameBuffer1.draw3D (-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, Colours::white);
        frameBuffer2.draw3D (-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, Colours::white);
        frameBuffer2.draw3D ( 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f, Colours::white);
        frameBuffer2.draw3D (-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, Colours::white);
    }

    void timerCallback()
    {
        rotation += delta;
        repaint();
    }

private:
    Image texture1, texture2;
    float rotation, delta;

    // Functions to create a couple of images to use as textures..
    static Image createImage1()
    {
        Image image (new OpenGLFrameBufferImage (Image::ARGB, 256, 256));

        Graphics g (image);

        g.fillAll (Colours::white.withAlpha (0.7f));
        g.drawImageWithin (ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize),
                           0, 0, image.getWidth(), image.getHeight(), RectanglePlacement::stretchToFit);

        drawRandomStars (g, image.getWidth(), image.getHeight());

        return image;
    }

    static Image createImage2()
    {
        Image image (new OpenGLFrameBufferImage (Image::ARGB, 128, 128));

        Graphics g (image);
        g.fillAll (Colours::darkred.withAlpha (0.7f));

        Path p;
        p.addStar (image.getBounds().getCentre().toFloat(), 11, image.getWidth() * 0.3f, image.getWidth() * 0.5f);

        g.setGradientFill (ColourGradient (Colours::blue,  image.getWidth() * 0.5f, image.getHeight() * 0.5f,
                                           Colours::green, image.getWidth() * 0.2f, image.getHeight() * 0.2f,
                                           true));
        g.fillPath (p);

        drawRandomStars (g, image.getWidth(), image.getHeight());

        return image;
    }

    static void drawRandomStars (Graphics& g, int w, int h)
    {
        Random r;
        for (int i = 10; --i >= 0;)
        {
            Path pp;
            pp.addStar (Point<float> (r.nextInt (w), r.nextInt (h)), r.nextInt (8) + 3, 10, 20, 0);
            g.setColour (Colours::pink.withAlpha (0.4f));
            g.fillPath (pp);
        }
    }
};


//==============================================================================
class OpenGLDemo  : public Component
{
public:
    OpenGLDemo()
        : Component ("OpenGL")
    {
        addAndMakeVisible (&canvas);
    }

    void resized()
    {
        canvas.setBounds (10, 10, getWidth() - 20, getHeight() - 50);
    }

private:
    DemoOpenGLCanvas canvas;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo);
};

//==============================================================================
Component* createOpenGLDemo()
{
    return new OpenGLDemo();
}

#endif
