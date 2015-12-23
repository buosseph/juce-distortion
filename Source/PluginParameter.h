#ifndef PLUGINPARAMETER_H_INCLUDED
#define PLUGINPARAMETER_H_INCLUDED

#include <functional>
#include <iomanip>
#include <sstream>

/** 
    Handles all parameter value mapping and conversion.
 
    JUCE handles all parameters as normalized float values, but parameters are eaiser
    to understand using their actual values. This class abstracts the handling and
    conversion between the parameters normalized and actual values.
 
    The parameter value is wrapped using JUCE's Atomic class to make the value thread
    safe.
 
    Requires C++11.
 */
class PluginParameter : public AudioProcessorParameter {
private:
    /// The parameter identifier, or id. This value is never changed after
    /// construction.
    Identifier identifier;
    
    /// The name of the parameter.
    String name;
    
    /// The suffix label, e.g. the unit of the actual value. This value is
    /// never changed after construction.
    String label;
    
    /// The normalized default parameter value, used by JUCE. This value is
    /// never changed after construction.
    float default_value;
    
    /// The normalized parameter value, used by JUCE.
    Atomic<float> value;
    
    /// The minimum actual parameter value. This value is never changed after
    /// construction.
    float actual_minimum;
    
    /// The maximum actual parameter value. This value is never changed after
    /// construction.
    float actual_maximum;
    
    /// The precision of the actual value displayed as text. The number of decimal
    /// places printed.
    int precision;
    
    /// The callback used after the parameter value has been updated. The argument
    /// passed to this callback is the actual parameter value.
    std::function<void(float)> callback;
    
public:
    /** 
        Creates a parameter from a normalized value.
     
        The given identifier must be unique and never change in code. It is used
        for keeping track of parameters when saving and loading state.
     
        The formatString is not checked, so if it is not valid the parameter value
        may not be formatted correctly, or even display, when calling getText().
     */
    PluginParameter (Identifier parameterId,
                     float defaultParameterValue,
                     const String& parameterName,
                     const String& parameterLabel = String::empty,
                     const int precision = 0,
                     std::function<void(float)> callback = nullptr)
    : identifier(parameterId),
    name(parameterName),
    label(parameterLabel),
    default_value(defaultParameterValue),
    precision(precision),
    callback(callback)
    {
        actual_minimum = 0.f;
        actual_maximum = 1.f;
        setValue(defaultParameterValue);
    }
    
    /** 
        Creates a parameter from an actual value and range.
     
        Creates a parameter from an actual value and range, the actual value
        will be set to the default and the normalized values will be calculated.
        The given identifier must be unique and never change in code. It is used
        for keeping track of parameters when saving and loading state.
     */
    PluginParameter(Identifier parameterId,
                    float actualDefaultValue,
                    float actualMinimum,
                    float actualMaximum,
                    const String& parameterName,
                    const String& parameterLabel = String::empty,
                    const int precision = 0,
                    std::function<void(float)> callback = nullptr)
    : identifier(parameterId),
    name(parameterName),
    label(parameterLabel),
    actual_minimum(actualMinimum),
    actual_maximum(actualMaximum),
    precision(precision),
    callback(callback)
    {
        default_value = calculateValue(actualDefaultValue);
        setValue(default_value);
    }
    
    /// Default destructor
    virtual ~PluginParameter() {};
    
    /// Returns the parameter identifier
    Identifier getIdentifier() const
    {
        return identifier;
    }

    /** Returns the normalized value corresponding to the given actual value.
     
        Returns the normalized value corresponding to the given value within the
        range of the actual parameter values.
     
        This class does not provide any set methods using actual parameter values,
        so use this to convert actual values to normalized values for usage in
        general JUCE code and to set the parameter value.
     
        One such case occurs when assigning slider values to parameters from an
        audio plugin editor.
     
        `parameter->setValueNotifyingHost(processor->calculateValue(actual_value));`
     
        This approach allows works so long as the range of the actual parameter values
        has been specified prior and if the given actual value is within the range.
     */
    float calculateValue(float actualValue) const
    {
        return (actualValue - actual_minimum) / (actual_maximum - actual_minimum);
    }
    
    /// Returns the actual value corresponding to the given normalized value within
    /// the range of the actual parameter values.
    float calculateActualValue(float value) const
    {
        return actual_minimum + (actual_maximum - actual_minimum) * value;
    }
    
    /// Returns the actual parameter value calculated from the parameter range and
    /// normalized value.
    float getActualValue() const
    {
        return calculateActualValue(value.get());
    }

    /// Returns the actual default parameter value calculated from the parameter
    /// range and normalized default value.
    float getActualDefaultValue() const
    {
        return calculateActualValue(default_value);
    }
    
    /// Returns the acutal minimum value of the parameter.
    float getActualMinimum() const
    {
        return actual_minimum;
    }
    
    /// Returns the actual maximum value of the parameter.
    float getActualMaximum() const
    {
        return actual_maximum;
    }

    /// Prints the values of all private variables. This is for debugging purposes.
    void printState()
    {
        std::cout
        << "PluginParameter\n"
        << "\tName: " << name
        << "\tLabel: " << label << "\n"
        << "\tValue: " << value.get() << "\tDefault: " << default_value
        << "\tActual: " << calculateActualValue(value.get()) << label
        << "\tDefault: " << calculateActualValue(default_value) << label
        << "\tRange: [" << actual_minimum << label
        << ", " << actual_maximum << label << "]\n";
    }

    // =======================================================
    // Inherited from AudioProcessorParameter
    // =======================================================
    
    /// Returns the value of the parameter.
    ///
    /// The return value is a normalized float within 0 - 1.f.
    float getValue() const override
    {
        return value.get();
    }
    
    /// Sets the value of the parameter.
    ///
    /// The new value must be a float within 0 - 1.f.
    void setValue(float newValue) override
    {
        value.set(newValue);
        if (callback != nullptr) {
            callback(getActualValue());
        }
    }

    /// Returns the default value of the parameter.
    ///
    /// The return value is a normalized float within 0 - 1.f.
    float getDefaultValue() const override
    {
        return default_value;
    }
    
    /// Returns the name of the parameter, made to fit a given stirng length.
    String getName(int maximumStringLength) const override
    {
        if (maximumStringLength < name.length()) {
            return name.substring(0, maximumStringLength - 1);
        }
        else {
            return name;
        }
        
    }
    
    /// Returns the label of a parameter.
    String getLabel() const override
    {
        return label;
    }
    
    /// Returns the actual parameter value as a string with a label.
    virtual String getText(float value, int stringLength) const override
    {
        std::ostringstream ss;
        ss  << std::fixed
            << std::setprecision(precision)
            << calculateActualValue(value);
        return ss.str();
    }
    
    /// Parses a string and returns interpreted normalized value.
    virtual float getValueForText(const String& text) const override
    {
        return text.getFloatValue();
    }
    
    // getNumStemps() is not implemented for continuous ranges, default implementation
    // calls getDefaultNumParameterSteps().
};


#endif  // PLUGINPARAMETER_H_INCLUDED
