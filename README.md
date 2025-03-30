# AmystAEGP
### A custom Adobe After Effects plugin that sends notifications through [Alertzy app](https://alertzy.app/) when your render is complete.

## ✨ Features  

✅ **Real-time render completion alerts** – Get notified via Alertzy app on your phone when your After Effects render finishes.  
✅ **Local REST API** – Send http requests to get effects and presets lists, apply effects, execute scripts and more.  

# 🚀 Installation  

1. **Download** the latest plugin version file from the [Releases](https://github.com/Amyst82/AmystAEGP/releases) section.  
2. **Install** the plugin by placing the entire folder in:  
   - **Windows:** `...\Adobe After Effects *\Support Files\Plug-ins\`  
   - **Mac:** `N/A yet`

## 🔧 Configuration  

Open the **settings.json** in the **...\Support Files\Plug-ins\Amyst** folder to set your alertzy account key and configure post-render actions:  

- **Notification** – Set to `true` if you want to get notification once your render is complete.  
- **PurgeSave** – `Purge` RAM cache (disk cache as well since 24.3) and `save your project`.
- **PurgeSaveQuit** – Purge RAM cache, save your project and `quit` After Effects.  
- **PurgeSaveShutdown** – Purge RAM cache, save your project, quit After Effects and `shutdown` your PC.

## 💡 REST API

I've managed to implement a couple of `GET` and `POST` endpoints. Local API server is built in the plugin and runs on `localhost` at `13377` port. You can make your own external FXConsole with the following endpoints:

- `GET` **/api/CheckAlive** – Returns `code 200` if After Effects in running.  
- `GET` **/api/ApplyEffect?name=** – Applyies an effect by its name to every selected layers. Returns `code 200` and nothing more.
- `GET` **/api/ApplyPreset?path=** – Applyies a preset by its path to every selected layers. Returns `code 200` and nothing more.
- `GET` **/api/FxList** – Returns all installed plugins and effects list. `text/plain`
- `GET` **/api/PresetsList** – Returns all user presets list. `text/plain`
- `POST` **/api/ExecuteScript** – Execute and JS script. `text/plain body`

### Examples
## csharp
```csharp
HttpWebRequest request = (HttpWebRequest)WebRequest.Create(https://localhost:13377/api/PresetsList);
request.AutomaticDecompression = DecompressionMethods.GZip | DecompressionMethods.Deflate;

using(HttpWebResponse response = (HttpWebResponse)request.GetResponse())
using(Stream stream = response.GetResponseStream())
using(StreamReader reader = new StreamReader(stream))
{
  return reader.ReadToEnd();
}
//Output: 
//D:\Programs\Adobe\Adobe After Effects 2022\Support Files\Presets\Backgrounds\Apparition.ffx
//D:\Programs\Adobe\Adobe After Effects 2022\Support Files\Presets\Backgrounds\Blocks.ffx
//D:\Programs\Adobe\Adobe After Effects 2022\Support Files\Presets\Backgrounds\Cinders.ffx
//...
```
## JavaScript
```js
var effectName = 'S_WarpChroma';
var url = 'https://localhost:13377/api/ApplyEffect?name=' + effectName;
let apiCall = new XMLHttpRequest();
apiCall.open("GET", url);
apiCall.send();
//Got response.
apiCall.onload = () => 
{
  if(apiCall.status != 200)
    return;
  //Do stuff
}
```
## csharp
```csharp
string apiUrl = "https://localhost:13377/api/ExecuteScript";
string scriptText = "alert('Hello from AEGP!');";
using (var client = new HttpClient())
{
    var content = new StringContent(scriptText, System.Text.Encoding.UTF8, "text/plain");         
    try 
    {
        HttpResponseMessage response = await client.PostAsync(apiUrl, content);
        string responseBody = await response.Content.ReadAsStringAsync();
                
        Console.WriteLine($"Status: {response.StatusCode}");
        Console.WriteLine($"Response: {responseBody}");
    }
    catch (HttpRequestException ex)
    {
        Console.WriteLine($"Error: {ex.Message}");
    }
}
```
![ezgif-1d2a0e18159e56](https://github.com/user-attachments/assets/ccc22588-ccc3-4789-ac6e-b041c4e0c853)

