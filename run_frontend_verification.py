import asyncio
from playwright.async_api import async_playwright

async def verify():
    async with async_playwright() as p:
        browser = await p.chromium.launch()
        page = await browser.new_page(viewport={"width": 1280, "height": 720})

        await page.goto("http://localhost:3000", wait_until="networkidle")
        await page.wait_for_timeout(3000)
        await page.screenshot(path="frontend_robot.png", full_page=False)

        # Scroll to halfway to show walk/character placement
        await page.evaluate("window.scrollTo(0, document.body.scrollHeight / 2)")
        await page.wait_for_timeout(1000)
        await page.screenshot(path="frontend_robot_scrolled.png", full_page=False)

        await browser.close()

asyncio.run(verify())
