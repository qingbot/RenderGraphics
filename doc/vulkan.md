# Vulkan

### Image,ImageView,FrameBuffer
* Image：原始图像数据的容器，不包含如何解释这些数据的信息。
* ImageView：对 Image 的一个视图，定义了如何访问和解释图像数据。
* Framebuffer：代表了特定 render pass 在渲染时使用的多个 ImageView。

当在 Vulkan 中执行渲染操作时，这三者通常会一起使用：首先有一个 Image 存储数据；接着创建一个 ImageView 来定义如何读取这个 Image；最后，创建一个包含一个或多个 ImageView 的 Framebuffer，用于渲染操作。在设置好这些资源之后，通过把 Framebuffer 绑定到 render pass，就可以开始执行渲染命令了。