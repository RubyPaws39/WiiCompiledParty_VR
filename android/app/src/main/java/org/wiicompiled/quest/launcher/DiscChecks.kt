package org.wiicompiled.quest.launcher

import java.security.MessageDigest
import java.util.Locale

/**
 * The PC installer's acceptance rules for a player's disc image, in one place:
 * Launcher/WiiCompiled.Setup.Windows/InputValidation.cs (formats, game ID) and
 * InstallerEngine.ValidateExtractedGame (the clean PAL main.dol and StaticR.rel).
 * The pins come from projects/mkwii/recomp.yml through BuildConfig.
 */
class DiscChecks(
    private val gameId: String,
    private val gameTitle: String,
    codeHashes: Map<String, String>,
) {
    private val codeHashes = codeHashes.toMap()

    constructor(gameId: String, dolSha256: String, relSha256: String) : this(
        gameId, "Mario Kart Wii PAL", mapOf(DOL_PATH to dolSha256, REL_PATH to relSha256)
    )

    init {
        require(gameId.matches(Regex("[A-Z0-9]{6}"))) { "Invalid disc game ID" }
        require(this.codeHashes.containsKey(DOL_PATH) && this.codeHashes.size <= 256) { "Invalid code manifest" }
        require(this.codeHashes.all { (path, hash) ->
            path.isNotEmpty() && !path.startsWith('/') && '\\' !in path &&
                path.split('/').none { it.isEmpty() || it == "." || it == ".." } &&
                hash.matches(Regex("[0-9a-fA-F]{64}"))
        }) { "Invalid code file path or SHA-256" }
    }

    data class Header(val gameId: String, val title: String, val extractedBytes: Long)

    /** Null when [displayName] is a format nod reads, otherwise the message to show. */
    fun extensionError(displayName: String?): String? {
        val extension = displayName?.substringAfterLast('.', "")?.lowercase(Locale.ROOT).orEmpty()
        return if (extension in SUPPORTED_EXTENSIONS) null else FORMAT_MESSAGE
    }

    fun compatibilityError(header: Header): String? {
        if (header.gameId.equals(gameId, ignoreCase = true)) {
            return null
        }
        return "This build supports $gameTitle ($gameId). The selected image is " +
            "${header.gameId} (${header.title}, ${region(header.gameId)})."
    }

    fun revisionError(readFile: (String) -> ByteArray): String? {
        for ((path, expected) in codeHashes) {
            val bytes = try {
                readFile(path)
            } catch (error: java.io.IOException) {
                return "The disc is missing or cannot read a required game file: $path."
            }
            if (!sha256(bytes).equals(expected, ignoreCase = true)) {
                return "The disc is $gameId but does not match the supported revision: $path. " +
                    "Use an unmodified copy of $gameTitle."
            }
        }
        return null
    }

    companion object {
        const val DOL_PATH = "sys/main.dol"
        const val REL_PATH = "files/rel/StaticR.rel"
        const val FST_PATH = "sys/fst.bin"

        private val SUPPORTED_EXTENSIONS = setOf("iso", "gcm", "gcz", "ciso", "wbfs", "wia", "rvz")
        private const val FORMAT_MESSAGE =
            "Select a complete Wii disc image in ISO, GCM, GCZ, CISO, WBFS, WIA, or RVZ format."

        /** NodDisc.header's "<game id>\n<title>\n<bytes>". */
        fun parseHeader(text: String): Header {
            val lines = text.split('\n')
            return Header(
                gameId = lines.getOrElse(0) { "" },
                title = lines.getOrElse(1) { "" },
                extractedBytes = lines.getOrNull(2)?.toLongOrNull() ?: 0L,
            )
        }

        fun region(gameId: String): String = when (gameId.getOrNull(3)) {
            'P' -> "PAL"
            'E' -> "NTSC-U"
            'J' -> "NTSC-J"
            'K' -> "Korea"
            'W' -> "Taiwan"
            null -> "Unknown"
            else -> gameId[3].toString()
        }

        fun sha256(bytes: ByteArray): String =
            MessageDigest.getInstance("SHA-256").digest(bytes).joinToString("") { "%02x".format(it) }
    }
}
