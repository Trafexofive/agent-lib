#!/usr/bin/env python3

import argparse
import requests
from bs4 import BeautifulSoup
import os
import sys
import time
import random
from urllib.parse import urljoin, quote_plus

# --- Configuration ---
# Headers to mimic a browser visit
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
}
# Base URL for Pinterest search (might change)
PINTEREST_SEARCH_URL = "https://www.pinterest.com/search/pins/?q="
# Be polite: delay between downloads (in seconds)
DOWNLOAD_DELAY = 1.0
# --- End Configuration ---

def create_directory(dir_path):
    """Creates the directory if it doesn't exist."""
    if not os.path.exists(dir_path):
        try:
            print(f"Creating directory: {dir_path}")
            os.makedirs(dir_path)
        except OSError as e:
            print(f"Error creating directory {dir_path}: {e}", file=sys.stderr)
            sys.exit(1)
    elif not os.path.isdir(dir_path):
        print(f"Error: {dir_path} exists but is not a directory.", file=sys.stderr)
        sys.exit(1)

def fetch_image_urls(query, num_images):
    """
    Attempts to fetch image URLs from Pinterest search results.
    NOTE: This is fragile and likely to break due to Pinterest changes
          and dynamic loading. May not find the requested number of images.
    """
    search_url = PINTEREST_SEARCH_URL + quote_plus(query)
    print(f"Searching Pinterest for: '{query}' (URL: {search_url})")
    image_urls = set() # Use a set to avoid duplicate URLs

    try:
        response = requests.get(search_url, headers=HEADERS, timeout=20)
        response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)

        soup = BeautifulSoup(response.text, 'html.parser')

        # --- THIS IS THE MOST FRAGILE PART ---
        # Find image tags. The specific tag, class, or structure
        # WILL change over time. This is just a guess based on potential structure.
        # You would need to inspect the current Pinterest HTML structure
        # using browser developer tools to find the right selectors.
        # Example: Look for divs containing images, then find 'img' tags within them.
        # Let's assume images are within divs with a specific (hypothetical) class
        # image_container_class = "XiG sLG zI7 iyn Hsu" # Example class - **FIND THE REAL ONE**
        # image_elements = soup.find_all('img', {"srcset": True}) # Often uses srcset
        image_elements = soup.find_all('img') # Broader search for <img> tags

        print(f"Found {len(image_elements)} potential image elements in initial HTML.")

        for img in image_elements:
            if len(image_urls) >= num_images:
                break

            # Try to get the highest resolution URL if possible (often in 'src' or 'srcset')
            src = img.get('src')
            srcset = img.get('srcset')
            high_res_url = None

            if srcset:
                # Take the last (often highest resolution) URL from srcset
                try:
                    high_res_url = srcset.strip().split(',')[-1].split()[0]
                except IndexError:
                    pass # Malformed srcset

            if not high_res_url and src:
                 # Filter out small icons or base64 encoded images
                if src.startswith('http') and 'avatar' not in src and 'data:image' not in src:
                     high_res_url = src

            # Further filter based on typical Pinterest image URL patterns (heuristic)
            if high_res_url and ('pinimg.com/originals' in high_res_url or 'pinimg.com/736x' in high_res_url or 'pinimg.com/564x' in high_res_url):
                 # Ensure URL is absolute
                high_res_url = urljoin(search_url, high_res_url)
                if high_res_url not in image_urls:
                    print(f"  Found potential image URL: {high_res_url}")
                    image_urls.add(high_res_url)


    except requests.exceptions.RequestException as e:
        print(f"Error fetching Pinterest search results: {e}", file=sys.stderr)
    except Exception as e:
         print(f"Error parsing Pinterest HTML: {e}", file=sys.stderr)
         print("This might be due to Pinterest changing their website structure.")

    if not image_urls:
        print("\nWarning: Could not find any image URLs.")
        print("This is common due to Pinterest's dynamic loading and anti-scraping measures.")
        print("Consider using a tool like Selenium or checking Pinterest's API policies.")


    return list(image_urls)[:num_images] # Return the requested number


def download_image(url, save_path):
    """Downloads a single image from a URL and saves it."""
    try:
        print(f"  Downloading: {url}")
        img_response = requests.get(url, headers=HEADERS, stream=True, timeout=20)
        img_response.raise_for_status()

        # Get filename from URL or generate one
        filename = os.path.basename(url.split('?')[0]) # Basic filename extraction
        if not filename or '.' not in filename: # Ensure it looks like a filename with extension
             filename = f"image_{random.randint(1000,9999)}.jpg" # Fallback

        full_path = os.path.join(save_path, filename)

        # Avoid overwriting existing files by adding a number if needed
        counter = 1
        base, ext = os.path.splitext(filename)
        while os.path.exists(full_path):
            full_path = os.path.join(save_path, f"{base}_{counter}{ext}")
            counter += 1


        with open(full_path, 'wb') as f:
            for chunk in img_response.iter_content(1024):
                f.write(chunk)
        print(f"  Saved to: {full_path}")
        return True
    except requests.exceptions.RequestException as e:
        print(f"  Error downloading {url}: {e}", file=sys.stderr)
        return False
    except IOError as e:
        print(f"  Error saving file {full_path}: {e}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"  An unexpected error occurred during download: {e}", file=sys.stderr)
        return False


def main():
    parser = argparse.ArgumentParser(description="Download a specified number of images from Pinterest based on a query.")

    parser.add_argument("-n", "--number", type=int, required=True,
                        help="Number of images to attempt to download.")
    parser.add_argument("-d", "--directory", type=str, required=True,
                        help="Directory name/path to store the downloaded images. Will be created if it doesn't exist.")
    parser.add_argument("query", type=str,
                        help="The search query for Pinterest.")

    args = parser.parse_args()

    if args.number <= 0:
        print("Error: Number of images (-n) must be positive.", file=sys.stderr)
        sys.exit(1)

    # Expand user path (like ~/)
    save_dir = os.path.expanduser(args.directory)

    print("--- Pinterest Image Downloader ---")
    print(f"Query:          '{args.query}'")
    print(f"Number to find: {args.number}")
    print(f"Save directory: {save_dir}")
    print("----------------------------------")
    print("IMPORTANT: Scraping Pinterest is unreliable and may violate their ToS.")
    print("This script might fail or only retrieve a few images.")
    print("----------------------------------")

    create_directory(save_dir)

    image_urls = fetch_image_urls(args.query, args.number)

    if not image_urls:
        print("No image URLs found or retrieved. Exiting.")
        sys.exit(1)

    print(f"\nFound {len(image_urls)} unique image URLs matching criteria. Attempting download...")

    downloaded_count = 0
    for i, url in enumerate(image_urls):
        if downloaded_count >= args.number:
            print(f"Reached target number of downloads ({args.number}).")
            break

        print(f"\nProcessing image {i+1}/{len(image_urls)}...")
        if download_image(url, save_dir):
            downloaded_count += 1
            # Polite delay
            time.sleep(DOWNLOAD_DELAY)
        else:
            print(f"  Skipping failed download: {url}")

    print("\n----------------------------------")
    print(f"Download process finished. Successfully downloaded {downloaded_count} images.")
    print("----------------------------------")

if __name__ == "__main__":
    main()
