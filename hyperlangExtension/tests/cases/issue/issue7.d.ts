export declare class AppSettings {
    private constructor();
    static getInstance(): AppSettings;
    getSetting(key: string): string | number | boolean | undefined;
  }