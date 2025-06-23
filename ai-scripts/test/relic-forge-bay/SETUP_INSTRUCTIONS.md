### Setup and Usage for Relic Forge Bay

This is a command-line tool. It does not run as a service.

**1. Make the tool executable:**

Ensure you are in the `relic-forge-bay` root directory. The first and only setup step is to make the wrapper script executable:

```bash
chmod +x forge
```

**2. How to Use:**

The `forge` command takes a plan file as its main argument and an optional output directory.

**To materialize a new project:**

```bash
# This command would create a new project directory based on the plan's relic_id
./forge <path_to_full_relic_plan.json> -o <output_directory>
```

**To modify an existing project:**

Use the included example plan to test the modification logic. First, create a dummy directory and file.

```bash
mkdir -p output/some-existing-relic/src
touch output/some-existing-relic/src/main.py

# Now, run the forge with the modification plan
./forge plans/example_modification_plan.json -o ./output
```

Check the `output/some-existing-relic` directory. You will see that `src/new_utils.py` has been created and `src/main.py` has been modified.