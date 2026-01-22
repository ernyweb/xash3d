package su.xash.engine.ui

import android.app.AlertDialog
import android.content.Context
import android.content.SharedPreferences
import android.view.ViewGroup
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView

class ResolutionDialog(context: Context) {
    private val sharedPref: SharedPreferences = context.getSharedPreferences("xash_settings", Context.MODE_PRIVATE)
    
    data class Resolution(val width: Int, val height: Int, val name: String)
    
    private val resolutions = arrayOf(
        Resolution(1920, 1080, "1920x1080 - Full HD (High-end)"),
        Resolution(1280, 720, "1280x720 - HD (Mid-range)"),
        Resolution(1024, 768, "1024x768 - XGA (Balanced)"),
        Resolution(960, 540, "960x540 - QHD (Medium)"),
        Resolution(854, 480, "854x480 - 854p (Lower resource)"),
        Resolution(800, 480, "800x480 - WVGA (Mobile standard)"),
        Resolution(768, 432, "768x432 - Half-HD (Compact)"),
        Resolution(720, 480, "720x480 - NTSC (Standard)"),
        Resolution(640, 480, "640x480 - VGA (Retro)"),
        Resolution(640, 360, "640x360 - nHD (Minimal)"),
        Resolution(480, 270, "480x270 - HVGA (Very low)"),
        Resolution(0, 0, "📝 Custom Resolution")  // Special marker for custom input
    )
    
    fun getSelectedResolution(): Pair<Int, Int> {
        val width = sharedPref.getInt("resolution_width", 1280)
        val height = sharedPref.getInt("resolution_height", 720)
        return Pair(width, height)
    }
    
    fun saveResolution(width: Int, height: Int) {
        sharedPref.edit().apply {
            putInt("resolution_width", width)
            putInt("resolution_height", height)
        }.apply()
    }
    
    fun showDialog(context: Context, onResolutionSelected: (Int, Int) -> Unit) {
        val names = resolutions.map { it.name }.toTypedArray()
        
        val (currentWidth, currentHeight) = getSelectedResolution()
        var selectedIndex = resolutions.indexOfFirst { 
            it.width == currentWidth && it.height == currentHeight 
        }
        if (selectedIndex < 0) selectedIndex = 1  // Default to 720p
        
        AlertDialog.Builder(context)
            .setTitle("Select Video Resolution")
            .setSingleChoiceItems(names, selectedIndex) { dialog, which ->
                if (which == resolutions.size - 1) {
                    // Custom resolution selected
                    dialog.dismiss()
                    showCustomResolutionDialog(context, onResolutionSelected)
                } else {
                    val selected = resolutions[which]
                    saveResolution(selected.width, selected.height)
                    onResolutionSelected(selected.width, selected.height)
                    dialog.dismiss()
                }
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
            }
            .show()
    }
    
    private fun showCustomResolutionDialog(context: Context, onResolutionSelected: (Int, Int) -> Unit) {
        // Create custom layout for width and height input
        val container = LinearLayout(context).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
            orientation = LinearLayout.VERTICAL
            setPadding(20, 20, 20, 20)
        }
        
        // Width label and input
        val widthLabel = TextView(context).apply {
            text = "Width (pixels):"
            textSize = 14f
        }
        val widthInput = EditText(context).apply {
            layoutParams = LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
            inputType = android.text.InputType.TYPE_CLASS_NUMBER
            hint = "e.g., 1920"
            setText(getSelectedResolution().first.toString())
        }
        
        // Height label and input
        val heightLabel = TextView(context).apply {
            text = "Height (pixels):"
            textSize = 14f
            setPadding(0, 16, 0, 0)
        }
        val heightInput = EditText(context).apply {
            layoutParams = LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
            inputType = android.text.InputType.TYPE_CLASS_NUMBER
            hint = "e.g., 1080"
            setText(getSelectedResolution().second.toString())
        }
        
        container.addView(widthLabel)
        container.addView(widthInput)
        container.addView(heightLabel)
        container.addView(heightInput)
        
        AlertDialog.Builder(context)
            .setTitle("Custom Resolution")
            .setView(container)
            .setPositiveButton("Apply") { dialog, _ ->
                try {
                    val width = widthInput.text.toString().toInt()
                    val height = heightInput.text.toString().toInt()
                    
                    if (width > 0 && height > 0 && width <= 7680 && height <= 4320) {
                        saveResolution(width, height)
                        onResolutionSelected(width, height)
                        dialog.dismiss()
                    } else {
                        android.widget.Toast.makeText(
                            context,
                            "Invalid resolution! Use 320-7680 width and 240-4320 height",
                            android.widget.Toast.LENGTH_SHORT
                        ).show()
                    }
                } catch (e: NumberFormatException) {
                    android.widget.Toast.makeText(
                        context,
                        "Please enter valid numbers",
                        android.widget.Toast.LENGTH_SHORT
                    ).show()
                }
            }
            .setNegativeButton("Cancel") { dialog, _ ->
                dialog.dismiss()
                // Reopen the main resolution dialog
                showDialog(context, onResolutionSelected)
            }
            .show()
    }
}
