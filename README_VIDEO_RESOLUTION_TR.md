# Xash3D Advanced Video Resolution System

Bu proje, Xash3D oyun motoruna kapsamlı bir video çözünürlük sistemi eklemektedir. Android ve diğer mobil platformlar da dahil olmak üzere, tüm platformlarda dinamik çözünürlük değiştirme özelliği sunar.

---

## 🎮 Özellikler

### Android için Yeni Özellikler

✨ **Dinamik Çözünürlük Değiştirme**
- Oyunu yeniden başlatmadan çözünürlüğü değiştirebilirsiniz
- 11 adet yaygın mobil çözünürlüğü destekler
- Özel çözünürlük girişi yapabilirsiniz (örn: 640x481)

✨ **Ekran Boyutu Farkındalığı**
- Cihazınızın ekranından daha büyük çözünürlükleri otomatik olarak filtreler
- Sadece uygun çözünürlükler menüde görülür

✨ **Geliştirilmiş Hata Yönetimi**
- Yararlı hata mesajları
- Geçersiz çözünürlükleri açıklı şekilde reddeder
- Mevcut çözünürlükleri listelemesi

### Masaüstü Platformları

Tüm mevcut özellikleri korur:
- Fullscreen/Windowed/Borderless modlar
- Birden fazla monitör desteği
- Ekran boyutunu otomatik algılama

---

## 🚀 Nasıl Kullanılır

### Konsol Komutu: `vid_mode`

#### Mevcut çözünürlükleri listele
```
vid_mode list
```

#### Hazır çözünürlüğü seç
```
vid_mode 0     # Birinci çözünürlük
vid_mode 5     # Altıncı çözünürlük
```

#### Özel çözünürlük gir
```
vid_mode 1920 1080    # 1920x1080
vid_mode 854 480      # 854x480
vid_mode 640 481      # 640x481 (istediğin gibi)
```

### Yapılandırma Dosyası (video.cfg)

Yapılandırma dosyasını projenin ana dizinine oluşturun:

```
# Çözünürlük ayarları
width "1280"
height "720"
fullscreen "1"

# Diğer ayarlar
gl_vsync "1"
vid_scale "1.0"
```

### Mevcut Ayarları Kontrol Et
```
vid_info
```

Görüntülenecek bilgiler:
- Mevcut çözünürlük
- Fullscreen durumu
- Render çözünürlüğü
- Video sürücüsü

---

## 📊 Hazır Çözünürlükler

### Android'de Mevcut Çözünürlükler

| Çözünürlük | Tür | En Boy |
|-----------|-----|---------|
| 1920x1080 | Full HD | 16:9 |
| 1280x720  | HD | 16:9 |
| 1024x768  | XGA | 4:3 |
| 960x540   | QHD | 16:9 |
| 854x480   | 854p | 16:9 |
| 800x480   | WVGA | 5:3 |
| 768x432   | Half-HD | 16:9 |
| 720x480   | NTSC | 3:2 |
| 640x480   | VGA | 4:3 |
| 640x360   | nHD | 16:9 |
| 480x270   | HVGA | 16:9 |

**Artı:** Herhangi bir özel çözünürlük! (minimum 320x200)

---

## 🎯 Android İçin Tavsiyeler

### Düşük Kapasiteli Cihazlar
```
vid_mode 640 480
```
- En düşük performans talebi
- Eski telefonlar için ideal

### Orta Seviye Cihazlar
```
vid_mode 1280 720
```
- İyi denge
- Çoğu modern telefon için uygun

### Yüksek Kapasiteli Cihazlar
```
vid_mode 1920 1080
```
- En iyi görsellik
- Yüksek performanslı telefonlar için

---

## ⚙️ Teknik Detaylar

### Neleri Değişti?

1. **SDL2, SDL3, SDL1 Arka Uçları Güncellendi**
   - Android'e uygun hazır çözünürlükler
   - Masaüstü platformları etkilenmedi

2. **Geliştirilmiş `vid_mode` Komutu**
   - Daha iyi hata mesajları
   - Daha kullanıcı dostu
   - Özel çözünürlük desteği

3. **Android Özgü İyileştirmeler**
   - Ekran boyutu kontrolü
   - Fullscreen varsayılanı
   - Dinamik yükleme

### Sistem Çalışması

```
1. Kullanıcı: vid_mode 1920 1080
   ↓
2. Motor: Geçerliliğini kontrol et (minimum 320x200)
   ↓
3. Motor: Ekran boyutunu kontrol et
   ↓
4. Motor: Çözünürlüğü değiştir
   ↓
5. Sonuç: Oyun yeni çözünürlüğe güncellendi
```

---

## 🔧 Sorun Giderme

### Çözünürlük değişmiyor

1. Doğru komutu kullandığını kontrol et:
   ```
   vid_mode 1920 1080
   ```

2. Desteklenen çözünürlükleri listele:
   ```
   vid_mode list
   ```

3. Geçerli ayarları kontrol et:
   ```
   vid_info
   ```

### "Geçersiz çözünürlük" hatası

- Minimum çözünürlük 320x200 olmalı
- Daha küçük değerler reddedilir
- Desteklenen çözünürlüklerden birini seç

### Çözünürlüğü başa döndürmek

Son başarılı çözünürlüğü geri yükler:
```
vid_mode 1280 720
```

---

## 📝 Dosya Yapısı

### Değiştirilen Dosyalar

- `engine/client/vid_common.c` - Komut geliştirmeleri
- `engine/platform/sdl2/vid_sdl2.c` - SDL2 arka ucu
- `engine/platform/sdl3/vid_sdl3.c` - SDL3 arka utu
- `engine/platform/sdl1/vid_sdl1.c` - SDL1 arka utu

### Oluşturulan Dosyalar

- `Documentation/video-resolution-system.md` - Detaylı kılavuz
- `video.cfg.example` - Örnek yapılandırma
- `video_resolution_guide.sh` - Hızlı referans
- `VIDEO_RESOLUTION_IMPLEMENTATION.md` - Teknik detaylar
- `DEPLOYMENT_SUMMARY.txt` - Dağıtım özeti

---

## 🌍 Platform Desteği

✅ **Android** - Tam destek, hazır çözünürlükler  
✅ **Windows** - Tam destek, mevcut özellikler korundu  
✅ **Linux** - Tam destek, mevcut özellikler korundu  
✅ **macOS** - Tam destek, mevcut özellikler korundu  
✅ **iOS** - Tam destek, hazır çözünürlükler  
✅ **Diğer Mobil** - Tam destek (XASH_MOBILE_PLATFORM ile)  

---

## 📖 Daha Fazla Bilgi

Detaylı bilgi için şu dosyaları okuyun:

1. **Kullanıcılar için:** `Documentation/video-resolution-system.md`
2. **Hızlı Referans:** `video_resolution_guide.sh`
3. **Yapılandırma Örneği:** `video.cfg.example`
4. **Geliştiriciler için:** `VIDEO_RESOLUTION_IMPLEMENTATION.md`

---

## 🎓 Örnekler

### Örnek 1: Oyunu HD çözünürlükte başlat

**Konsol üzerinde:**
```
vid_mode 1280 720
```

**Yapılandırma dosyasında:**
```
# video.cfg dosyasında
width "1280"
height "720"
fullscreen "1"
```

### Örnek 2: 854x480 özel çözünürlüğü kullan

```
vid_mode 854 480
```

### Örnek 3: Cihazda en iyi çözünürlüğü bul

```
vid_mode list        # Tüm seçenekleri gör
vid_mode 0          # İlkini dene
vid_info            # Sonucu kontrol et
```

---

## ✨ Yenilikler Özeti

| Özellik | Öncesi | Sonrası |
|---------|--------|---------|
| Android'de Çözünürlük Değişimi | ❌ Yok | ✅ Var |
| Dinamik Yükleme | ❌ Gerekli Restart | ✅ Anında |
| Özel Çözünürlük | ⚠️ Kısıtlı | ✅ Tam |
| Hata Mesajları | ⚠️ Minimal | ✅ Detaylı |
| Hazır Çözünürlükler | ❌ Android'de yok | ✅ 11 adet |
| Ekran Boyutu Kontrolü | ❌ Yok | ✅ Var |
| Dokumentasyon | ⚠️ Az | ✅ Kapsamlı |

---

## 🚀 Başlamak İçin

1. **Projeyi derle:**
   ```bash
   ./waf configure
   ./waf build_android  # Android için
   ```

2. **Çözünürlükleri listele:**
   ```
   vid_mode list
   ```

3. **İstediğin çözünürlüğü seç:**
   ```
   vid_mode 1920 1080
   ```

4. **Ayarları kontrol et:**
   ```
   vid_info
   ```

---

## 💡 İpuçları

- Performans sorunları yaşıyorsan daha düşük çözünürlük dene
- `vid_mode list` komutu tüm seçenekleri gösterir
- `video.cfg` dosyası başlangıç ayarlarını belirler
- Android her zaman fullscreen modunda çalışır
- Masaüstü özellikler değişmedi, geriye uyumlu

---

## 📞 Destek

Sorular veya sorunlar için:
1. Dokumentasyonu oku: `Documentation/video-resolution-system.md`
2. Hızlı rehbere bak: `video_resolution_guide.sh`
3. Örnek yapılandırmayı kontrol et: `video.cfg.example`

---

**Son Güncelleme:** 22 Ocak 2026  
**Sürüm:** 1.0  
**Durum:** ✅ Hazır Kullanım İçin
