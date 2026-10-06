import asyncio
from playwright.async_api import async_playwright

async def main():
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        page = await browser.new_page()

        # Listen for console events
        page.on("console", lambda msg: print(f"Browser console: {msg.type}: {msg.text}"))
        page.on("pageerror", lambda err: print(f"Browser error: {err}"))

        # Go to the local server
        await page.goto("http://localhost:3000", wait_until="networkidle")

        # Wait a bit to ensure 3D scene is loaded
        await page.wait_for_timeout(3000)

        await browser.close()

asyncio.run(main())
