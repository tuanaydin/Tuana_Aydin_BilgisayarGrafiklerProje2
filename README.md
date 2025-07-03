# Tuana_Aydin_BilgisayarGrafiklerProje2
**Ad Soyad:** Tuana Aydın  
**Numara:** 1316200027  
**Ders:** Bilgisayar Grafikleri  
**Teslim Tarihi:** 30.06.2025
**Öğrenci Tarafında Teslim Tarihi:** 03.07.2025
**Proje Teması:** Kütüphane Sahnesi 

Bilgisayar Grafikleri dersi kapsamında verilen projede özgün bir temaya veya kısa bir hikâyeye dayanan, etkileşimli veya statik bir 3D sahne oluşturulması beklenmektedir. Bu sahnede OpenGL kullanılarak en az üç adet nesne (örneğin bir karakter, yapı, çevre öğesi) tanımlanmalı ve her biri bilgisayar grafikleri temel prensiplerine göre modellenmelidir. Sahne, dönüşümler, ışıklandırma ve shader kullanımı ile zenginleştirilmelidir.
Bu amaç doğrultusunda OpenGL 3.3 versiyonu kullanılarak Kütüphane temasındaki bir sahne tasarlanmıştır.
 
🎯 **Proje Teması**
Bu projede müze teması kullanarak hem birçok gerçek hayatta yer alan sanat eserine yer verdim hem de gerçekçi bir görüntü olmasını hedefledim.Projenin konusu yeni malzemelerin getirildiği sanat müzesine yanlışlıkla kutunun içinde gelen kedi karakterinin aldığı kararlar ile şekilleniyor.

**Kullanılan Kütüphaneler**
- GLFW: Pencere oluşturma ve giriş kontrolleri için
- GLAD: OpenGL fonksiyonlarına erişim
- GLM: Matris ve vektör hesaplamaları
- OpenGL: Grafik çizimi ve sahne yönetimi(Windows'un içinde yer almaktadır.)
- stb_image.h: Texture yüklenmesi
- stb_easy_font.h: Basit UI yazılarının eklenmesi.(Maalesef projede etkin olarak kullanılamadı.) 

Not:GLAD yerine GLEW kütüphanesi de kullanılabilir.
Not: Yukarıda yer alan kütühpaneler dışında cmath,string.h gibi daha birçok yapıdan faydalanılmıştır.Listede en önemli olanlara yer verilmiştir.

Kütüphaneler aşağıda yer alan sitelerden alınmıştır:
-https://www.glfw.org/download   ---->GLFW
-https://glad.dav1d.de/  ---->GLAD
-https://github.com/g-truc/glm  ---->GLM

**Kullanılan Programlama Dilleri:**
-C++  
-GLSL (Shaderların yazımında kullanılmıştır.)


** 🛠️ Teknik Özellikler:**
- Asansör animasyonu
- 3D koordinat sisteminde modelleme
- GLM ile model, dünya, görünüm dönüşümleri
- Phong aydınlatma modeli (ambient, diffuse, specular)
- Shader programlama: vertex ve fragment shader kullanımı
- Kamera hareketi: WASD + fare kontrolü
- Doku Uygulaması
- Karakter Animasyonu

**Projeyi Çalıştırmak İçin Gerekli Adımlar:**
1. Proje klasörünü Visual Studio 2022 ile açın.
2. GLFW, GLEW ve GLM kütüphanelerinin bağlı olduğundan emin olun.
3. `main.cpp` dosyasını çalıştırarak sahneye erişebilirsiniz. Shader dosyaları main.cpp dosyasının içinde yer almaktadır.

Ek Not: Not: Shader'lar main.cpp içerisinde tanımlanmıştır. Kodun daha okunabilir olması için vertex ve fragment shader'ları ayrı dosyalar halinde (`vertex_shader.glsl`, `fragment_shader.glsl`) oluşturabilirsiniz. Bu durumda dosya yollarını main.cpp içerisinde doğru şekilde belirtmeyi unutmayın.

**📷 Ekran Görüntüleri**



**Proje Dosya Yapısı**
📁 Bilgisayar-Grafikleri_2025
│
├── README.md
├── main.cpp
├── screenshots/
│   ├── kamera1.png
│   ├── kamera2.png
│   └── kamera3.png
├── report/
│   └── ProjeRaporu.pdf
